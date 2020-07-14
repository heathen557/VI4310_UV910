#include "camerasetting_dialog.h"
#include "ui_camerasetting_dialog.h"

CameraSetting_Dialog::CameraSetting_Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CameraSetting_Dialog)
{
    ui->setupUi(this);

    init_parameterFile();
}

//加载ini
void CameraSetting_Dialog::init_parameterFile()
{
    QSettings configSetting("setting.ini", QSettings::IniFormat);

    float focal_length = configSetting.value("camera/focal_length").toFloat();
    float camera_diff = configSetting.value("camera/camera_diff").toFloat();
    int integrate_num = configSetting.value("camera/integrate_num").toInt();

    ui->focalLength_lineEdit->setText(QString::number(focal_length));
    ui->camera_diff_lineEdit->setText(QString::number(camera_diff));
    ui->integrate_number_lineEdit->setText(QString::number(integrate_num));

}

CameraSetting_Dialog::~CameraSetting_Dialog()
{
    delete ui;
}


//!
//! \brief CameraSetting_Dialog::on_ok_pushButton_clicked
//! 确定的槽函数
void CameraSetting_Dialog::on_ok_pushButton_clicked()
{
    float focal_length_float = ui->focalLength_lineEdit->text().toFloat();    //焦距
    float tx_rx_camera_diff = ui->camera_diff_lineEdit->text().toFloat();     //tx /rx 间距
    int integrate_num = ui->integrate_number_lineEdit->text().toInt();             //积分次数

    if(integrate_num>4095)
    {
        QMessageBox::information(NULL,QStringLiteral("提示"),QStringLiteral("设置的积分次数过大，请重新输入！"));
        return;
    }


    //  积分次数 需要写入寄存器
    QString integrateStr = QString("%1").arg(integrate_num,3,16,QChar('0')).toUpper();
    qDebug()<<"integrateStr = "<<integrateStr;
    QString highByte = integrateStr.mid(0,1) + "0";
    QString lowByte = integrateStr.mid(1,2);
    qDebug()<<"highByte="<<highByte<<"  lowByte="<<lowByte;

    QString addressStr = "08";
    emit write_I2C_integrate_signal(addressStr,lowByte);
    Sleep(10);
    addressStr = "07";
    emit write_I2C_integrate_signal(addressStr,highByte);






    emit alter_focal_integrate_signal(focal_length_float,tx_rx_camera_diff,integrate_num);     //焦距 和 TX-RX间距 积分次数

    QSettings configSetting("setting.ini", QSettings::IniFormat);
    configSetting.setValue("camera/focal_length",focal_length_float);
    configSetting.setValue("camera/camera_diff",tx_rx_camera_diff);
    configSetting.setValue("camera/integrate_num",integrate_num);

    this->hide();
}


//!
//! \brief CameraSetting_Dialog::on_cancel_pushButton_clicked
//!取消的槽函数
void CameraSetting_Dialog::on_cancel_pushButton_clicked()
{
    this->hide();
}
