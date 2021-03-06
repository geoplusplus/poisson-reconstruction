/*
Copyright (c) 2006, Michael Kazhdan and Matthew Bolitho
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of
conditions and the following disclaimer. Redistributions in binary form must reproduce
the above copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided with the distribution. 

Neither the name of the Johns Hopkins University nor the names of its contributors
may be used to endorse or promote products derived from this software without specific
prior written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE IMPLIED WARRANTIES 
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
DAMAGE.
*/

#include <cstring>
#include <cfloat>

template<class T>
template<class T2>
Vector<T2> SparseSymmetricMatrix<T>::operator*(Vector<T2> const& V) const {
	Vector<T2> R(Rows());
	for(int i = 0; i != Rows(); ++i) {
		for(int ii = 0; ii != rowSizes_[i]; ++ii) {
			MatrixEntry<T> e = m_ppElements[i][ii];
			R[i] += e.Value * V[e.N];
			R[e.N] += e.Value * V[i];
		}
	}
	return R;
}

template<class T>
template<class T2>
void SparseSymmetricMatrix<T>::Multiply(Vector<T2> const& in, Vector<T2>& out, bool addDCTerm,
		int threads) const {
	std::vector<std::vector<T2> > OutScratch(threads);
	for(int t = 0; t < threads; ++t)
		OutScratch[t].assign(in.Dimensions(), 0);
#pragma omp parallel for num_threads(threads) schedule(static)
	for(int i = 0; i < Rows(); ++i) {
		std::vector<T2>& outs = OutScratch[omp_get_thread_num()];
		if(addDCTerm) {
			for(int ii = 0; ii != rowSizes_[i]; ++ii) {
				MatrixEntry<T> e = m_ppElements[i][ii];
				outs[i] += e.Value * in[e.N];
				outs[e.N] += e.Value * in[i];
			}
		} else {
			T2 acc = 0;
			for(int ii = 0; ii != rowSizes_[i]; ++ii) {
				MatrixEntry<T> e = m_ppElements[i][ii];
				acc += e.Value * in[e.N];
				outs[e.N] += e.Value * in[i];
			}
			outs[i] += acc;
		}
	}
	T2 dcTerm = 0;
	if(addDCTerm) {
#pragma omp parallel for num_threads(threads) reduction(+ : dcTerm)
		for(int i = 0; i < Rows(); ++i) dcTerm += in[i];
		dcTerm /= out.Dimensions();
	}

#pragma omp parallel for num_threads(threads) schedule(static)
	for(size_t i = 0; i < out.Dimensions(); ++i) {
		out[i] = dcTerm;
		for(int t = 0; t < threads; ++t) out[i] += OutScratch[t][i];
	}
}

template<class T>
template<class T2>
int SparseSymmetricMatrix<T>::Solve(SparseSymmetricMatrix<T> const& A, Vector<T2> const& b, int iters,
		Vector<T2>& x, T2 eps, bool reset, int threads, bool addDCTerm) {
	eps *= eps;
	int dim = b.Dimensions();
	if(threads < 1) threads = 1;
	if(reset) x = Vector<T2>(dim);

	Vector<T2> r(dim);
	A.Multiply(x, r, addDCTerm, threads);

	Vector<T2> d(dim);
	double delta_new = 0;
#pragma omp parallel for num_threads(threads) reduction(+ : delta_new)
	for(int i = 0; i < dim; ++i) {
		d[i] = r[i] = b[i] - r[i];
		delta_new += r[i] * r[i];
	}

	if(delta_new < eps) {
		std::cerr << "[WARNING] Initial residual too low: " << delta_new << " < " << eps << std::endl;
		return 0;
	}
	double delta_0 = delta_new;
	int ii;
	for(ii = 0; ii != iters && delta_new > eps * delta_0; ++ii) {
		Vector<T2> q(dim);
		A.Multiply(d, q, addDCTerm, threads);
		double dDotQ = 0;
#pragma omp parallel for num_threads(threads) reduction(+ : dDotQ)
		for(int i = 0; i < dim; ++i) dDotQ += d[i] * q[i];
		T2 alpha = delta_new / dDotQ;
		double delta_old = delta_new;
		delta_new = 0;

		if(ii % 50 == 49) {
#pragma omp parallel for num_threads(threads)
			for(int i = 0; i < dim; ++i) x[i] += d[i] * alpha;
			A.Multiply(x, r, addDCTerm, threads);
#pragma omp parallel for num_threads(threads) reduction(+ : delta_new)
			for(int i = 0; i < dim; ++i) {
				r[i] = b[i] - r[i];
				delta_new += r[i] * r[i];
				x[i] += d[i] * alpha;
			}
		} else {
#pragma omp parallel for num_threads(threads) reduction(+ : delta_new)
			for(int i = 0; i < dim; ++i) {
				r[i] -= q[i] * alpha;
				delta_new += r[i] * r[i];
				x[i] += d[i] * alpha;
			}
		}

		T2 beta = delta_new / delta_old;
#pragma omp parallel for num_threads(threads)
		for(int i = 0; i < dim; ++i) d[i] = r[i] + d[i] * beta;
	}
	return ii;
}

template<class T>
T SparseSymmetricMatrix<T>::Norm(size_t Ln) const {
	T N = 0;
	for(size_t i = 0; i != rowSizes_.size(); ++i)
		for(int j = 0; j != rowSizes_[i]; ++j)
			N += std::pow(m_ppElements[i][j].Value, (T)Ln);
	return std::pow(N, (T)1.0 / Ln);
}
