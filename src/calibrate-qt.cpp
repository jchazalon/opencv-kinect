//#include <opencv2/highgui/highgui.hpp>


#include <QtGui>
#include <QtWidgets>

// OpenCV includes

#include <opencv2/core.hpp>
#include "capture-cv.hpp"
#include "calibration-utils.hpp"
#include "utils.hpp"

//using namespace cv;


class QCalibrationApp : public QMainWindow
{
    public:

    QCalibrationApp(QWidget* parent = nullptr);



    void peek_frame() {
        capture.next_loop_event();
    }

    void recompute_homography() {
        if (rgb->pixmap().isNull())
            return;

        int coordinates[8];
        for (int i = 0; i < 4; ++i) {
            coordinates[2*i] = controls[i]->scenePos().x();
            coordinates[2*i+1] = controls[i]->scenePos().y();
        }
        this->H = unwrap_estimate(coordinates, rgb->pixmap().width(), rgb->pixmap().height());
    }

    private:

    struct QControl : public QGraphicsRectItem {
        QCalibrationApp* parent;

        QControl(QCalibrationApp* parent, int x, int y) : QGraphicsRectItem(0, 0, 10, 10) {
            setBrush(QBrush(Qt::red));
            setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges);
            this->parent = parent;
            this->setPos(x, y);
        }

        QVariant itemChange(GraphicsItemChange change, const QVariant& value) override {
            if (change == ItemPositionHasChanged) {
                parent->recompute_homography();
            }
            return QGraphicsRectItem::itemChange(change, value);
        }
    };

    QGraphicsView* lview;
    QGraphicsView* cview;
    QGraphicsView* rview;
    QGraphicsScene* lscene;
    QGraphicsScene* cscene;
    QGraphicsScene* rscene;
    QGraphicsPixmapItem* rgb;
    QGraphicsPixmapItem* unwrapped;
    QGraphicsPixmapItem* depth;
    QTimer* timer;
    CVKinectCapture capture;

    // Control
    QControl* controls[4];

    cv::Mat H; // Homography matrix

};

QCalibrationApp::QCalibrationApp(QWidget* parent) : QMainWindow(parent)
{
    rgb = new QGraphicsPixmapItem();
    unwrapped = new QGraphicsPixmapItem();
    depth = new QGraphicsPixmapItem();

    lview = new QGraphicsView();
    cview = new QGraphicsView();
    rview = new QGraphicsView();
    lscene = new QGraphicsScene();
    cscene = new QGraphicsScene();
    rscene = new QGraphicsScene();

    controls[0] = new QControl(this, 0, 0);
    controls[1] = new QControl(this, 630, 0);
    controls[2] = new QControl(this, 630, 470);
    controls[3] = new QControl(this, 0, 470);

    lscene->addItem(rgb);
    for (int i = 0; i < 4; ++i) {
        lscene->addItem(controls[i]);
    }
    cscene->addItem(unwrapped);
    rscene->addItem(depth);
    
    lview->setScene(lscene);
    cview->setScene(cscene);
    rview->setScene(rscene);

    lview->setMinimumSize(640, 480);
    cview->setMinimumSize(640, 480);
    rview->setMinimumSize(640, 480);

    auto central = new QWidget();
    auto layout = new QHBoxLayout(central);
    layout->addWidget(lview);
    layout->addWidget(cview);
    layout->addWidget(rview);
    this->setCentralWidget(central);



    capture.set_rgb_callback([&](cv::Mat& input, uint32_t timestamp) {
        QImage image(input.data, input.cols, input.rows, input.step, QImage::Format_RGB888);
        rgb->setPixmap(QPixmap::fromImage(image));
        lscene->setSceneRect(image.rect());

        cv::Mat unwraped = (H.empty()) ? input : unwrap(input, H);
        QImage unwrapped_image(unwraped.data, unwraped.cols, unwraped.rows, unwraped.step, QImage::Format_RGB888);
        unwrapped->setPixmap(QPixmap::fromImage(unwrapped_image));
        cscene->setSceneRect(unwrapped_image.rect());
    });

    capture.set_depth_callback([&](cv::Mat& _depth, uint32_t timestamp) {
        static auto cmap = get_cmap();

        //std::cout << "Depth callback..." << std::endl;
        cv::Mat_<uint16_t> depth16 = _depth;

        cv::Mat depth_rgb(depth16.rows, depth16.cols, CV_8UC3);
        depth16.forEach([&](uint16_t& pixel, const int position[2]) {
            rgb8 color = cmap[pixel];
            depth_rgb.at<cv::Vec3b>(position[0], position[1]) = cv::Vec3b(color.r, color.g, color.b);
        });

        if (!H.empty()) {
            depth_rgb = unwrap(depth_rgb, H);
        }


        QImage image(depth_rgb.data, depth_rgb.cols, depth_rgb.rows, depth_rgb.step, QImage::Format_RGB888);
        depth->setPixmap(QPixmap::fromImage(image));
        rscene->setSceneRect(image.rect());
    });

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &QCalibrationApp::peek_frame);
    timer->start(10);

    capture.start();
}





int main(int argc, char** argv)
{
    // Create a QT application with a window and side-by-side RGB and Depth panel
    QApplication app(argc, argv);

    QCalibrationApp win;
    win.show();

    return app.exec();
}