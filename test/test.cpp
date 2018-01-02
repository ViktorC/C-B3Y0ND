/*
 * test.cpp
 *
 *  Created on: Dec 10, 2017
 *      Author: Viktor Csomor
 */

#include <Activation.h>
#include <DataPreprocessor.h>
#include <Eigen/Dense>
#include <iostream>
#include <Layer.h>
#include <Loss.h>
#include <Matrix.h>
#include <NeuralNetwork.h>
#include <Optimizer.h>
#include <RegularizationPenalty.h>
#include <vector>
#include <Vector.h>
#include <WeightInitialization.h>

typedef double Scalar;

static Scalar func(Scalar x, Scalar y, Scalar z) {
	return x * x * x + x * y + 3 * z - x * y * z + 10;
};

int main() {
	std::cout << "Number of threads: " << Eigen::nbThreads() << std::endl;
	cppnn::Matrix<Scalar> data(21 * 21 * 21, 3);
	unsigned row = 0;
	for (Scalar i = -2.0; i <= 2.01; i += .2) {
		for (Scalar j = -1.0; j <= 1.01; j += .1) {
			for (Scalar k = -3.0; k <= 3.01; k += .3) {
				data(row, 0) = i;
				data(row, 1) = j;
				data(row, 2) = k;
				row++;
			}
		}
	}
	cppnn::Matrix<Scalar> obj(data.rows(), 1);
	for (int i = 0; i < obj.rows(); i++) {
		obj(i,0) = func(data(i,0), data(i,1), data(i,2));
	}
	cppnn::NormalizationDataPreprocessor<Scalar> preproc(true);
//	cppnn::PCAPreprocessor<Scalar> preproc(true, true, 0.99);
	preproc.fit(data);
	preproc.transform(data);
	cppnn::ReLUWeightInitialization<Scalar> init;
	cppnn::XavierWeightInitialization<Scalar> f_init;
	cppnn::ReLUActivation<Scalar> act;
	cppnn::IdentityActivation<Scalar> f_act;
	std::vector<cppnn::Layer<Scalar>*> layers(3);
	layers[0] = new cppnn::FCLayer<Scalar>(data.cols(), 100, init, act);
	layers[1] = new cppnn::FCLayer<Scalar>(100, 50, init, act);
	layers[2] = new cppnn::FCLayer<Scalar>(50, 1, f_init, f_act);
	cppnn::FFNeuralNetwork<Scalar> nn(layers);
	nn.init();
	cppnn::QuadraticLoss<Scalar> loss;
	cppnn::L2RegularizationPenalty<Scalar> reg(1e-3);
	cppnn::AdaMaxOptimizer<Scalar> opt(loss, reg, 32, .8);
	std::cout << nn.to_string() << std::endl << std::endl;
//	std::cout << opt.verify_gradients(nn, data, obj) << std::endl;
	opt.train(nn, data, obj, 200);
	Scalar x = -0.31452;
	Scalar y = 0.441;
	Scalar z = -1.44579;
	Scalar out = func(x, y, z);
	cppnn::Matrix<Scalar> in(1, 3);
	in(0,0) = x;
	in(0,1) = y;
	in(0,2) = z;
	preproc.transform(in);
	std::cout << "Estimate: " << nn.infer(in) << std::endl;
	std::cout << "Actual value: " << out << std::endl;
	return 0;
};
