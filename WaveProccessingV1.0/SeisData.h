#pragma once

#ifndef KVL_SEISDATA_H_
#define KVL_SEISDATA_H_

#include <armadillo>
#include <memory>
#include <vector>
#include <algorithm>

//assist in getting seis info.

namespace kvl
{
	//
	// This class maintain a 2D-float matrix ptr(Eigen::MatrixXf).
	// 
	// It provides all the informations of the exploration seismic data,
	// such as the number of tracks(n), the number of sampling points(m), 
	// sampling interval(dt), ...etc
	//

	struct SubInfo
	{
		using u32_t = unsigned int;
		SubInfo() {}
		SubInfo(u32_t n, u32_t m, float dt, float dx):
			n(n), m(m), dt(dt), dx(dx) {}

	    u32_t n;
		u32_t m;
		float dt;
		float dx;
	};

	class SeisData
	{
		using mat2f_ptr = std::shared_ptr<arma::fmat>;
		using vecf_ptr = std::shared_ptr<std::vector<float>>;
		using size_type = unsigned int;
	public:
		SeisData() :
			dt_(0.0), dx_(0.0), maxColsPtr_(vecf_ptr(nullptr)), sdataPtr_(nullptr) {}

		//assume dt = 1ms, dx = 2.0m. User should set it by real data.
		explicit SeisData(mat2f_ptr sdataPtr, float dt = 0.001, float dx = 1.0) :
			sdataPtr_(sdataPtr), dt_(dt), dx_(dx) {
			initMaxCols();
		}

		SeisData(const SeisData& orig) :
			sdataPtr_(orig.sdataPtr_), dt_(orig.dt_),
			dx_(orig.dx_), maxColsPtr_(orig.maxColsPtr_) {}

		SeisData& operator=(const SeisData&);

		// Be careful! altho ugh it's const,
		// but you can modify the real data easily by the ptr;
		mat2f_ptr getSeisDataPtr() const { return sdataPtr_; }
		std::shared_ptr<arma::cx_fmat> fft2Seis();
		std::shared_ptr<arma::cx_fmat> fft2Seis(arma::uword rows, arma::uword cols);
		std::shared_ptr<arma::cx_fmat> fft2Inv(std::shared_ptr<arma::cx_fmat> fft2DataPtr);

		float getDt() const { return dt_; }
		float getDx() const { return dx_; }
		size_type getN() const { return sdataPtr_->n_cols; }

		size_type getM() const { return sdataPtr_->n_rows; }
		//return max element fabs of every col.
		vecf_ptr getMaxColsPtr() const { return maxColsPtr_; }
		float getMaxData();

		void setDt(float dt) { dt_ = dt; }
		void setDx(float dx) { dx_ = dx; }

		SubInfo getSubInfo() const
		{
			return SubInfo(getN(), getM(), dt_, dx_);
		}

		~SeisData() {}

	private:
		float dt_;
		float dx_;   //interval of tracks
		vecf_ptr  maxColsPtr_;
		mat2f_ptr sdataPtr_;

		void initMaxCols();  //max data in every track
	};

	inline void SeisData::initMaxCols()
	{
		auto n = getN(), m = getM();
		maxColsPtr_ = std::make_shared<std::vector<float>>(n);
		for (size_type i = 0; i < n; ++i)
		{
			float smax = 0.0;
			for (size_type j = 0; j < m; ++j)
				if (fabs((*sdataPtr_)(j, i)) > smax)
					smax = fabs((*sdataPtr_)(j, i));
			(*maxColsPtr_)[i] = smax;
		}
	}

	inline float SeisData::getMaxData()
	{
		return *std::max_element(maxColsPtr_->begin(), maxColsPtr_->end());
	}

	inline SeisData& SeisData::operator=(const SeisData& rhs)
	{
		sdataPtr_ = rhs.sdataPtr_;
		maxColsPtr_ = rhs.maxColsPtr_;
		dt_ = rhs.dt_;
		dx_ = rhs.dx_;
		return *this;
	}
}

#endif // !KVL_SEISDATA_H_