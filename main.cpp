#include <QApplication>
#include "MainWindow.cpp" // أو MainWindow.h حسب الهيكلة

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // إنشاء واجهة المستخدم وإظهارها
    // MainWindow window;
    // window.show();

    return app.exec();
}
