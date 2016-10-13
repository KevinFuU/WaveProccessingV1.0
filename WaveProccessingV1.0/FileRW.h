#pragma once

#pragma once

#ifndef KV_FILEREAD_H_
#define KV_FILEREAD_H_

#include "SeisData.h"
#include <stdio.h>
#include <string>
#include <QtCore/QString>
class QString;

namespace kvl
{
	class FileRW final
	{
		using mat2f_ptr = std::shared_ptr<arma::fmat>;

	public:
		FileRW() {}
		explicit FileRW(const char* filename);
		explicit FileRW(std::string& filename);

#ifdef QSTRING_H
		explicit FileRW(QString &filename);
#endif

		//IO shouldn't be copyable
		FileRW(const FileRW&) = delete;
		FileRW& operator=(const FileRW&) = delete;

		~FileRW() {}

		bool readSeisFile(const char*);
		bool readSeisFile(std::string&);
#ifdef QSTRING_H
		bool readSeisFile(QString &);
#endif

		bool toZxqFile(const char*);
		bool toZxqFile(std::string&);
#ifdef QSTRING_H
		bool toZxqFile(QString &);
#endif

		bool toCsvFile(const char *);

		SeisData& getData() { return sdata_; }

	private:
		SeisData sdata_;

		const char judgeHow2Read(FILE *&fpin);
		void readSega(FILE *&fpin);
		void readZxq(FILE *&fpin);
		void readSeg2(FILE *&fpin);
	};
}

#endif // !KV_FILEREAD_H_