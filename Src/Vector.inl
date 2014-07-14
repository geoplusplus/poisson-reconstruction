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

template<class T>
Vector<T>& Vector<T>::operator*=(const T& A) {
	for(size_t i = 0; i != data_.size(); ++i) data_[i] *= A;
	return *this;
}

template<class T>
Vector<T>& Vector<T>::operator/=(const T& A) {
	for(size_t i = 0; i != data_.size(); ++i) data_[i] /= A;
	return *this;
}

template<class T>
Vector<T>& Vector<T>::operator+=(Vector<T> const& V) {
	for(size_t i = 0; i != data_.size(); ++i) data_[i] += V.data_[i];
	return *this;
}

template<class T>
T Vector<T>::Norm(size_t Ln) const {
	T N = 0;
	for(size_t i = 0; i != data_.size(); ++i)
		N += std::pow(data_[i], (T)Ln);
	return std::pow(N, (T)1.0 / Ln);
}
