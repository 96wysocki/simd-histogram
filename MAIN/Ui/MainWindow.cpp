#include "MainWindow.hpp"
#include <QDesktopWidget>
#include <QDockWidget>
#include <QFileDialog>
#include <QMessageBox>
#include "Ui/ControlWidget.hpp"
#include "Ui/HistogramsWidget.hpp"
#include "Ui/LogWidget.hpp"
#include "Ui/ImagePreviewWidget.hpp"
#include "ImageLabel.hpp"

MainWindow::MainWindow(QWidget * parent)
	: QMainWindow(parent),
	  m_number_of_threads(std::thread::hardware_concurrency()) {

	setWindowTitle(tr("Histogram calculator"));

	// fix initial value of threads if hardware_concurrency value is unavailable
	if (m_number_of_threads == 0) {
		m_number_of_threads = 1;
	}

	// set initial window size
	this->resize(QDesktopWidget().availableGeometry(this).size() * 0.7);

	_init_widgets();
	_connect_signals();
}

void MainWindow::_init_widgets() {

	// histograms widget (central)
	m_histograms_widget = new HistogramsWidget{ this };
	setCentralWidget(m_histograms_widget);

	// image preview dock widget
	m_image_preview_widget = new ImageLabel{ this };
	m_image_preview_dock = new QDockWidget{ tr("Image preview"), this };
	m_image_preview_dock->setWidget(m_image_preview_widget);
	m_image_preview_dock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);
	m_image_preview_dock->setFeatures(QDockWidget::DockWidgetMovable);
	m_image_preview_dock->setVisible(false);
	m_image_preview_dock->setSizePolicy(QSizePolicy::Minimum,
	                                    QSizePolicy::MinimumExpanding);
	addDockWidget(Qt::RightDockWidgetArea, m_image_preview_dock);

	// control widget
	m_control_widget = new ControlWidget{ this };
	auto control_dock { new QDockWidget{ tr("Control panel"), this } };
	control_dock->setWidget(m_control_widget);
	control_dock->setFeatures(QDockWidget::DockWidgetMovable);
	control_dock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, control_dock);

	// logger widget
	m_logger_widget = new LogWidget{ this };
	auto logger_dock { new QDockWidget{ tr("Log"), this } };
	logger_dock->setWidget(m_logger_widget);
	logger_dock->setFeatures(QDockWidget::DockWidgetMovable);
	addDockWidget(Qt::RightDockWidgetArea, logger_dock);
}

void MainWindow::_connect_signals() {
	connect(
		m_control_widget,
		&ControlWidget::load_image_pressed,
		[=] {
			_load_image_pressed();
		}
	);

	connect(
		m_control_widget,
		&ControlWidget::calculate_histogram_pressed,
		[=] {
			m_histograms_widget->clear();
			m_control_widget->lock_generate_histograms_button(true);
			m_processing_service.calculate_histograms(m_calculation_policy, m_number_of_threads);
		}
	);

	connect(
		m_control_widget,
		&ControlWidget::calculation_policy_changed,
		[=](calculation_policy policy) {
			this->m_calculation_policy = policy;
			const QString current_policy { ( policy == calculation_policy::cpp ) ? "C++" : "assembly" };
			this->m_logger_widget->
			      print_message(tr("Strategy changed (") + current_policy + ")");
		}
	);

	connect(
		&m_processing_service,
		&ProcessingService::channels_ready,
		[=] {
			m_logger_widget->print_message(tr("Image loaded successfully"));
			m_control_widget->lock_load_image_button(false);
			m_control_widget->lock_generate_histograms_button(false);
			m_control_widget->lock_run_benchmark_from_image_button(false);
			m_image_preview_dock->setVisible(true);
		}
	);

	connect(
		&m_processing_service,
		&ProcessingService::histograms_calculated,
		[=](std::chrono::duration<double> duration, histogram_bundle_t histograms) {
			const QString message {
				"Histograms calculated successfully (using "
				+ QString::number(m_number_of_threads)
				+ " task"
				+ ( m_number_of_threads == 1 ? "" : "s" )
				+ ", elapsed: "
				+ QString::number(milliseconds(duration).count())
				+ " ms)."
			};
			m_logger_widget->print_message(message);

			m_histograms_widget->plot(histograms);
			m_control_widget->lock_generate_histograms_button(false);
			m_control_widget->lock_load_image_button(false);
		}
	);

	connect(&m_processing_service, &ProcessingService::benchmark_complete,
	        [this](benchmark_result_t result) {
		        m_logger_widget->print_message(
			        tr("C++: ") +
			        QString::number(milliseconds(result.cpp_elapsed).count()) +
			        tr(" ms<br/>Assembly: ") + QString::number(milliseconds(result.asm_elapsed).count()) + tr(" ms"));
	        });

	connect(
		m_control_widget,
		&ControlWidget::benchmark_iterations_number_changed,
		[=](int number_of_iterations) {
			m_benchmark_number_of_iterations = number_of_iterations;
			m_logger_widget->print_message(tr("Iterations: ") + QString::number(number_of_iterations));
		}
	);

	connect(
		m_control_widget,
		&ControlWidget::number_of_tasks_changed,
		[=](int number_of_threads) {
			m_number_of_threads = number_of_threads;
			m_logger_widget->print_message(tr("Tasks: ") + QString::number(number_of_threads));
		}
	);

	connect(m_control_widget,
	        &ControlWidget::run_benchmark_from_image_pressed, [this] {
		        this->m_processing_service.run_benchmark(
			        this->m_number_of_threads,
			        this->m_benchmark_number_of_iterations);
		        m_logger_widget->print_message(tr("Running benchmark (iterations: ")
		                                       + QString::number(this->m_benchmark_number_of_iterations)
		                                       + tr(")..."));
	        });
}

void MainWindow::_load_image_pressed() {
	m_control_widget->lock_load_image_button(true);
	m_control_widget->lock_generate_histograms_button(true);

	QString filename = QFileDialog::getOpenFileName(
		this, "Load image", QDir::homePath(),
		tr("Image files (*.png *.jpg *.jpeg *.bmp)"));

	// filename is empty if user dismissed file dialog
	if (!filename.isEmpty()) {
		QImage image { filename };
		image = image.convertToFormat(QImage::Format_RGB32);
		m_control_widget->lock_load_image_button(false);

		if (!image.isNull()) {
			m_control_widget->lock_load_image_button(true);

			const QString loaded_image_info_message {
				tr("Loading image \"") + filename + "\" (" +
				QString::number(image.width()) + " x " +
				QString::number(image.height()) + ") " +
				tr("Total ") + QString::number(image.width() * image.height()) + tr(" pixels")
			};

			m_logger_widget->print_message(loaded_image_info_message);

			m_processing_service.load_channels(image);
			m_image_preview_widget->set_image(image);
		} else {
			QMessageBox msg_box {
				QMessageBox::NoIcon, QString { "Error" },
				QString { "File unsupported." }, QMessageBox::Ok, this
			};
			msg_box.exec();
		}
	} else {
		m_control_widget->lock_load_image_button(false);
		if (m_processing_service.channels_are_loaded()) {
			m_control_widget->lock_generate_histograms_button(false);
		}
	}
}