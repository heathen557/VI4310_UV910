#include "blackgetcount.h"

extern int m_nDevID;

blackGetCount::blackGetCount(QObject *parent) : QObject(parent)
{
    qDebug()<<"blackGetCount  thread has start";
    fileIndex = 0;
}

//十进制 16　16
bool blackGetCount::write_IIC_slot(QString reg ,QString value)
{
    UINT uAddr = 216;
    UINT uReg = reg.toInt(NULL,16);
    UINT uValue =  value.toInt(NULL,16);

    if (WriteSensorReg(uAddr, uReg, uValue, 0, m_nDevID) == FALSE)
    {
        QMessageBox::warning(NULL,QStringLiteral("提示"),QStringLiteral("写入I2C出错!"));
        return false;
    }
    QString str_log = "[Write I2C]:I2C_addr="+QString("%1").arg(uAddr,2,16,QChar('0')).toUpper() + ",Reg="+QString("%1").arg(uReg,2,16,QChar('0')).toUpper() + ",Value="+QString("%1").arg(uValue,2,16,QChar('0')).toUpper();
}

//16进制  读取   返回值 16进制数
QString blackGetCount::read_IIC_slot(QString reg)
{
    int iRet = 0;
    UINT uAddr = 216;
    UINT uReg = reg.toInt(NULL,16);
    USHORT uValue;
    QString sValue;

    iRet = ReadSensorReg(uAddr,uReg,&uValue,0,m_nDevID);
    if(iRet != DT_ERROR_OK)
    {
        return -1;
    }
    sValue = QString("%1").arg(uValue,2,16,QChar('0')).toUpper();
    QString str_log = "[Read I2C]:I2C_addr="+QString("%1").arg(uAddr,2,16,QChar('0')).toUpper() + ",Reg="+QString("%1").arg(uReg,2,16,QChar('0')).toUpper() + ",Value="+QString("%1").arg(uValue,2,16,QChar('0')).toUpper();

    return sValue;
}

//开始读取pixel
void blackGetCount::start_blackGetCount_slot(QString filePath,QString InteTime,int saveFrame,QStringList rowAndCol_list)
{
    qDebug()<< "start_blackGetCount_slot(QString filePath,QString InteTime,int saveFrame) has come"<<saveFrame;


    int len = rowAndCol_list.length();
    qDebug()<<" blackGetCount::len =  "<<len;
//    for(int i=0; i<len; i+=2)
//    {
//        qDebug()<<"row = "<<rowAndCol_list[i] <<"   col="<<rowAndCol_list[1+i];
//    }


    saveFilePath = filePath;
    int cloudIndex = 0;   //0-4799
    QString regTmp;
    QString valueTmp;
    QString resStr;
    int frameIndex= 0;

    //1、 写入 1d ,val = 0

    int row;
    int col;
    for(frameIndex=0;frameIndex<saveFrame;frameIndex++ )
    {
        for(int pointIndex=0; pointIndex<len; pointIndex+=2 )
        {
            row = rowAndCol_list[pointIndex].toInt();
            col = rowAndCol_list[1+pointIndex].toInt();

            if(row<30)
            {
                row = 29-row;
            }else
            {
                row = row - 30;
                col = col + 80;
            }


            regTmp = "1D";
            valueTmp = "00";
            if(!write_IIC_slot(regTmp,valueTmp))
            {
                qDebug()<<"write_IIC_slot(1D,0)  error";
            }

            //2、  写入行  reg：0x40   val:00-1d
            regTmp = "40";
            valueTmp = QString("%1").arg(row,2,16,QChar('0'));
            if(!write_IIC_slot(regTmp,valueTmp))
            {
                qDebug()<<"write_IIC_slot(40,row)  error";
            }
            //3、 写入列 reg:0x 3f   val:00-9f
            regTmp = "3F";
            valueTmp = QString("%1").arg(col,2,16,QChar('0'));
            if(!write_IIC_slot(regTmp,valueTmp))
            {
                qDebug()<<"write_IIC_slot(3F,COL)  error";
            }

            //4、使能3C  3C 1+X
            regTmp = "3C";
            valueTmp = "1"+InteTime.right(1);
            if(!write_IIC_slot(regTmp,valueTmp))
            {
                qDebug()<<"write_IIC_slot(3C,17)  error";
            }

            //度寄存器49 直到为1
            Sleep(10);
            while(1)
            {
                regTmp = "49";
                resStr = read_IIC_slot(regTmp);
                if(1 == resStr.toInt(NULL,16))
                {
                    break;
                }
                Sleep(10);
            }
            //5 读取数据  4A
            regTmp = "4A";
            resStr = read_IIC_slot(regTmp);

            int imageRow =0;
            int imageCol = 0;
            if(col<80)
            {
                imageRow =  29- row;
                imageCol = col;
            }else
            {
                imageRow = 30 + row;
                imageCol = col-80;
            }

            qDebug()<<" imageRow = "<<imageRow<<"  imageCol="<<imageCol;

            if(resStr.isEmpty())
            {
                qDebug()<<" read ERROR row= "<<row<<"  col="<<col;
            }

            cloudIndex = imageRow*80+imageCol;
            int resCount = resStr.toInt(NULL,16);
//          getCountStr[cloudIndex] = resStr+"\n";
            getCountStr[cloudIndex] = QString::number(resCount)+"\n";

            //关闭计数  3c 写
            //6、使能3C  3C 1+X
            regTmp = "3C";
            valueTmp = "0"+InteTime.right(1);
            if(!write_IIC_slot(regTmp,valueTmp))
            {
                qDebug()<<"write_IIC_slot(3C,07)  error";
            }

        }//一帧数据已经接收完毕

        for(int i=0;i<4800;i++)
        {
            savedString.append(getCountStr[i]);
        }
        writeTXT(savedString,frameIndex);
        savedString.clear();
        emit sendSavedFrame_signal(frameIndex+1);
    }




//    for(frameIndex=0;frameIndex<saveFrame;frameIndex++ )
//    {
//        for(int row=0; row<30;row++)
//        {
//            for(int col=0;col<160;col++)
//            {

//                regTmp = "1D";
//                valueTmp = "00";
//                if(!write_IIC_slot(regTmp,valueTmp))
//                {
//                    qDebug()<<"write_IIC_slot(1D,0)  error";
//                }

//                //2、  写入行  reg：0x40   val:00-1d
//                regTmp = "40";
//                valueTmp = QString("%1").arg(row,2,16,QChar('0'));
//                if(!write_IIC_slot(regTmp,valueTmp))
//                {
//                    qDebug()<<"write_IIC_slot(40,row)  error";
//                }
//                //3、 写入列 reg:0x 3f   val:00-9f
//                regTmp = "3F";
//                valueTmp = QString("%1").arg(col,2,16,QChar('0'));
//                if(!write_IIC_slot(regTmp,valueTmp))
//                {
//                    qDebug()<<"write_IIC_slot(3F,COL)  error";
//                }

//                //4、使能3C  3C 1+X
//                regTmp = "3C";
//                valueTmp = "1"+InteTime.right(1);
//                if(!write_IIC_slot(regTmp,valueTmp))
//                {
//                    qDebug()<<"write_IIC_slot(3C,17)  error";
//                }

//                //度寄存器49 直到为1
//                Sleep(10);
//                while(1)
//                {
//                    regTmp = "49";
//                    resStr = read_IIC_slot(regTmp);
//                    if(1 == resStr.toInt(NULL,16))
//                    {
//                        break;
//                    }
//                    Sleep(10);
//                }
//                //5 读取数据  4A
//                regTmp = "4A";
//                resStr = read_IIC_slot(regTmp);

//                int imageRow =0;
//                int imageCol = 0;
//                if(col<80)
//                {
//                    imageRow =  29- row;
//                    imageCol = col;
//                }else
//                {
//                    imageRow = 30 + row;
//                    imageCol = col-80;
//                }

//                if(resStr.isEmpty())
//                {
//                    qDebug()<<" read ERROR row= "<<row<<"  col="<<col;
//                }

//                cloudIndex = imageRow*80+imageCol;
//                int resCount = resStr.toInt(NULL,16);
////                getCountStr[cloudIndex] = resStr+"\n";
//                getCountStr[cloudIndex] = QString::number(resCount)+"\n";

//                //关闭计数  3c 写
//                //6、使能3C  3C 1+X
//                regTmp = "3C";
//                valueTmp = "0"+InteTime.right(1);
//                if(!write_IIC_slot(regTmp,valueTmp))
//                {
//                    qDebug()<<"write_IIC_slot(3C,07)  error";
//                }


//            }
//        }//一帧数据已经接收完毕

//        for(int i=0;i<4800;i++)
//        {
//            savedString.append(getCountStr[i]);
//        }
//        writeTXT(savedString,frameIndex);
//        savedString.clear();
//        emit sendSavedFrame_signal(frameIndex+1);
//    }

}





//按照标识写文件
// input:text:写入文本的内容
// numOfFile：第几个文件
void blackGetCount::writeTXT(QString text, int index)
{
    QString sFilePath = saveFilePath + QString::number(index)+".txt";
    QFile file(sFilePath);
//    file.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Append);
    file.open(QIODevice::WriteOnly|QIODevice::Text);

    QTextStream out(&file);
    out<<text.toLocal8Bit()<<endl;
    file.close();

//    emit send_savedFileIndex_signal(index);
}



