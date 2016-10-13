#include "SeisData.h"

namespace kvl
{
	using namespace arma;

	std::shared_ptr<arma::cx_fmat> SeisData::fft2Seis(uword rows, uword cols)
	{
		fmat zeroMat(size(*sdataPtr_), fill::zeros);
		cx_fmat sdata(*sdataPtr_, zeroMat);
		cx_fmat fft2Data = fft2(sdata, rows, cols);
		return std::make_shared<cx_fmat>(fft2Data);
	}

	std::shared_ptr<arma::cx_fmat> SeisData::fft2Seis()
	{
		auto rows = sdataPtr_->n_rows;
		auto cols = sdataPtr_->n_cols;
		return fft2Seis(rows, cols);
	}

	std::shared_ptr<arma::cx_fmat> 
		SeisData::fft2Inv(std::shared_ptr<arma::cx_fmat> fft2DataPtr)
	{
		cx_fmat invMat = ifft2(*fft2DataPtr);
		return std::make_shared<cx_fmat>(invMat);
	}
}