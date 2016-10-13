#include "FileRW.h"
#include <string.h>
#include <stdlib.h>
#include <exception>
#include <QtCore/QString>
#include <QDebug>
namespace kvl
{
	using namespace arma;

	FileRW::FileRW(const char *filename)
	{
		//case can't read file, reset sdata_
		if (!readSeisFile(filename)) { sdata_ = SeisData(); }
	}

	FileRW::FileRW(std::string& filename)
	{
		//case can't read file, reset sdata_
		if (!readSeisFile(filename)) { sdata_ = SeisData(); }
	}

	FileRW::FileRW(QString &filename)
	{
		//case can't read file, reset sdata_
		if (!readSeisFile(filename)) { sdata_ = SeisData(); }
	}

	const char FileRW::judgeHow2Read(FILE *&fpin)
	{
		fseek(fpin, 0, 0);
		char smark;
		fread(&smark, 1, 1, fpin);
		return smark;
	}

	void FileRW::readZxq(FILE *&fpin)
	{
		short n, m; float dt;
		fseek(fpin, 14, 0);
		fread(&n, 2, 1, fpin);//��ȡ�����¼�ĵ���n
		fread(&m, 2, 1, fpin);//��ȡ�����¼�Ĳ�������m
		fread(&dt, 4, 1, fpin);//��ȡ�����¼�Ĳ������dt

		auto sdataPtr = std::make_shared<fmat>(m, n);
		auto oneChSdata = new float[m]; //may fail.
		fseek(fpin, 22, 0);
		for (int i = 0; i < n; ++i)
		{
			fread(oneChSdata, sizeof(float), m, fpin);
			for (int j = 0; j < m; ++j)
				(*sdataPtr)(j, i) = oneChSdata[j];
		}

		sdata_ = SeisData(sdataPtr, dt);
		delete[] oneChSdata;
		fclose(fpin);
	}

	void FileRW::readSega(FILE *&fpin)
	{
		short n, m; float dt;
		fseek(fpin, 4, 0);  fread(&n, 2, 1, fpin);//��ȡ�����¼�ĵ���n
		fseek(fpin, 26, 0);  fread(&m, 2, 1, fpin);//��ȡ�����¼�Ĳ�������m
		fseek(fpin, 14, 0);  fread(&dt, 4, 1, fpin);//��ȡ�����¼�Ĳ������dt

		auto lData = new long[m];
		auto sdataPtr = std::make_shared<fmat>(m, n);
		fseek(fpin, 512, 0);
		for (int i = 0; i < n; ++i)
		{
			fread(lData, sizeof(long), m, fpin);
			for (int j = 0; j < m; ++j)
				(*sdataPtr)(j, i) = (float)lData[j];
		}
		sdata_ = SeisData(sdataPtr, dt);
		fclose(fpin);
		delete[] lData;
	}

	void FileRW::readSeg2(FILE *&fpin)
	{
		short i, n, m, lm2; long lm1, lm3;
		qDebug() << sizeof(long);
		fseek(fpin, 6, 0);  fread(&n, 2, 1, fpin); //��ȡ����n
		fseek(fpin, 32, 0);  fread(&lm1, 4, 1, fpin);//��ȡ��1���ĵ�ͷλ��lm1
		fseek(fpin, lm1 + 2, 0);  fread(&lm2, 2, 1, fpin);//��ȡ��ͷ���ֽ���lm2
		char strDT[16] = "", SI[16] = "SAMPLE_INTERVAL";
		for (i = 0; i < lm2 - 15; ++i)  //search the mark's pos.
		{
			fseek(fpin, lm1 + i, 0);  fread(strDT, 15, 1, fpin);
			if (strcmp(SI, strDT) == 0)
				break;
		}

		float dt; char tt[11] = "", mark;
		fseek(fpin, lm1 + i + 15, 0);  fread(tt, 10, 1, fpin);//��ȡ����������ַ��ı�
		dt = (float)atof(tt); //���ı�ת��Ϊ����
		fseek(fpin, lm1 + 8, 0);  fread(&lm3, 4, 1, fpin); //��ȡ��������lm3,���ǳ�����
		m = (short)lm3;//��lm3�͸������ͱ���m
		fseek(fpin, lm1 + 12, 0);  fread(&mark, 1, 1, fpin);//��ȡ�����������ͱ�־��mark
		if (mark != 4 && mark != 1 && mark != 2)
		{
			fclose(fpin);
			throw std::invalid_argument("Wrong SEG2 format.\n");
			return;
		}

		auto sdataPtr = std::make_shared<fmat>(m, n); //����n(��)���Ĵ�ŵ�ַ�Ŀռ�
		auto lData = new long[m];//�ø�ʽ������Ϊ������,����m�����ݴ洢�ռ�,��ʱ���ڶ�ȡ1������
		auto oneChSdat = new float[m];//����m��float���ݴ洢�ռ�,��ʱ���ڶ�ȡ1������
		auto stInt = new short[m];//����m��short���ݴ洢�ռ�,��ʱ���ڶ�ȡ1������
		for (int k = 0; k < n; ++k)
		{
			fseek(fpin, 32 + k * 4, 0);  fread(&lm1, 4, 1, fpin);//��ȡ��k���ĵ�ͷλ��lm1
			fseek(fpin, lm1 + 2, 0);  fread(&lm2, 2, 1, fpin);//��ȡ��k���ĵ�ͷ�ֽ���lm2
			fseek(fpin, lm1 + lm2, 0); //���ļ��ڲ�ָ�� �Ƶ� ��k�������������ʼλ��
			if (mark == 4)
			{
				fread(oneChSdat, 4, m, fpin);
				for (short j = 0; j < m; ++j)
					(*sdataPtr)(j, k) = oneChSdat[j];
			}
			else if (mark == 2)
			{
				fread(lData, 4, m, fpin);
				for (short j = 0; j < m; ++j)
					(*sdataPtr)(j, k) = (float)lData[j];
			}
			else
			{
				fread(stInt, 2, m, fpin);
				for (int j = 0; j < m; ++j)
					(*sdataPtr)(j, k) = (float)stInt[j];
			}
		}
		sdata_ = SeisData(sdataPtr, dt);
		fclose(fpin);
		delete[] stInt; delete[] oneChSdat; delete[] lData;
	}

	bool FileRW::readSeisFile(const char *filename)
	{

		FILE *fpin;
		if (fpin = fopen(filename, "rb"))
		{
			auto mark = judgeHow2Read(fpin);
			switch (mark)
			{
			case 221:
				readSega(fpin); return true;
			case 85:
				readSeg2(fpin); return true;
			case 84:
				readZxq(fpin); return true;
			default:
				throw std::invalid_argument("Can't recognise seismic file type.");
			}
		}
		else
		{
			throw std::logic_error("Can't open the file");
		}
	}

	bool FileRW::readSeisFile(std::string &str)
	{
		auto filename = str.c_str();
		return readSeisFile(filename);
	}

	//decrease the dependence of lib.
#ifdef QSTRING_H
	bool FileRW::readSeisFile(QString &qstr)
	{
		QByteArray ba = qstr.toLocal8Bit();
		const char* filename = ba.data();
		return readSeisFile(filename);
	}
#endif

	bool FileRW::toZxqFile(const char *filename)
	{
		FILE *fpout;
		if (fpout = fopen(filename, "wb"))
		{
			fseek(fpout, 0, 0);					fwrite("TX", 2, 1, fpout);
			float shot = 0.0;					fwrite(&shot, 4, 1, fpout);
			float oft = 0.0;					fwrite(&oft, 4, 1, fpout);
			float dx = 1.0;						fwrite(&dx, 4, 1, fpout);
			short n = (short)(sdata_.getN());	fwrite(&n, 2, 1, fpout);
			short m = (short)(sdata_.getM());	fwrite(&m, 2, 1, fpout);
			float dt = sdata_.getDt();			fwrite(&dt, 4, 1, fpout);
			auto sdataPtr = sdata_.getSeisDataPtr();
			auto oneChSdata = new float[m];
			for (int i = 0; i < n; ++i)
			{
				for (int j = 0; j < m; ++j)
					oneChSdata[j] = (*sdataPtr)(j, i);
				fwrite(oneChSdata, sizeof(float), m, fpout);
			}
			fclose(fpout);
			return true;
		}
		else
		{
			throw std::logic_error("Can't open the file");
		}
	}

	bool FileRW::toZxqFile(std::string &str)
	{
		auto filename = str.c_str();
		return toZxqFile(filename);
	}

#ifdef QSTRING_H
	bool FileRW::toZxqFile(QString &qstr)
	{
		QByteArray ba = qstr.toLocal8Bit();
		const char* filename = ba.data();
		return toZxqFile(filename);
	}
#endif

	bool FileRW::toCsvFile(const char *filename)
	{
		FILE *fpout;
		if (fpout = fopen(filename, "w"))
		{
			auto n = sdata_.getN();
			auto m = sdata_.getM();
			auto sdataPtr = sdata_.getSeisDataPtr();
			for (unsigned int i = 0; i < n; ++i)
			{
				for (unsigned int j = 0; j < m; ++j)
				{
					if (i < n - 1)
						fprintf(fpout, "%.2f,", (*sdataPtr)(j, i));
					else
						fprintf(fpout, "%.2f", (*sdataPtr)(j, i));
				}
				fprintf(fpout, "\n");
			}
			fclose(fpout);
			return true;
		}
		else
		{
			throw std::logic_error("can't write csv file.");
		}
	}
}