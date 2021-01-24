#include <QApplication>
#include <QPushButton>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QPushButton button ("Hello world!");
    button.show();

    return a.exec(); // .exec starts QApplication and related GUI, this line starts 'event loop'    
}