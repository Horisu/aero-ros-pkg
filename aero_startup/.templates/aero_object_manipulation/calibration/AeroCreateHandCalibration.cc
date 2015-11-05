/*
 * ThirdParty : libsvm/svm.h from https://github.com/cjlin1/libsvm
 */

#include "libsvm/svm.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <cmath>
#include <algorithm>

// @brief Parameter configurations
void SetParameters(svm_parameter &_param, double _C,
		   double _gamma, double _epsilon)
{
  _param.svm_type = EPSILON_SVR;
  _param.kernel_type = RBF;
  _param.eps = 1e-3;
  _param.cache_size = 100;
  _param.gamma = _gamma; 
  _param.C = _C;
  _param.p = _epsilon;
  _param.shrinking = 0;
  _param.probability = 0;
};

// @brief Learn and evaluate SVR
std::vector<double> SVR(svm_model *_model,
			svm_parameter _param,
			std::vector<points> _data_learn,
			std::vector<double> _y_learn,
			std::vector<points> _data_test,
			std::vector<double> _y_validation)
{
  std::cout << "Regression...\n";

  svm_problem problem;
  problem.l = _data_learn.size();
  problem.y = new double[problem.l];
  problem.x = new svm_node * [problem.l];

  svm_node *dat = new svm_node[nodes * problem.l];

  for (unsigned int i = 0; i < problem.l; ++i)
  {
    /* implement here dat */
    problem.x[i] = &dat[i * nodes];
    problem.y[i] = _y_learn[i];
  }

  _model = svm_train(&problem, &_param);

  std::cout << "Test...\n";

  std::vector<double> y_test(_data_test.size());
  svm_node testdat[nodes];

  for (unsigned int i = 0; i< _data_test.size(); ++i)
  {
    /* implement here test_dat */
    y_test[i] = svm_predict(_model, testdat);
  }

  // Evaluate the trained values

  std::vector<double> accuracy(_y_validation.size() + 1);
  double error_sum = 0.0;

  for (unsigned int i = 0; i < _y_validation.size(); ++i)
  {
    double err = fabs(y_test[i] - _y_validation[i]);
    accuracy[i + 1] = err;
    error_sum += err;
  }
  accuracy[0] = error_sum;

  delete[] dat;
  delete[] problem.x;
  delete[] problem.y;

  return accuracy;
};

// @brief Train from best found parameters and save learned model
void SaveModel(std::string _filename,
	       svm_model *_model, svm_parameter _param,
	       std::vector<points> _data_learn, std::vector<double> _y)
{
  std::cout << "Regression...\n";

  svm_problem problem;
  problem.l = _data_learn.size();
  problem.y = new double[problem.l];
  problem.x = new svm_node * [problem.l];

  svm_node *dat = new svm_node[nodes * problem.l];

  for (unsigned int i = 0; i < problem.l; ++i)
  {
    /* implement here dat */
    problem.x[i] = &dat[i * nodes];
    problem.y[i] = _y[i];
  }

  _model = svm_train(&problem, &_param);

  svm_save_model(_filename.c_str(), _model);

  delete[] dat;
  delete[] problem.x;
  delete[] problem.y;
};

// @brief Read input data file and output data file
void Read(std::string _file_data, std::string _file_y,
	  std::vector<points> &data, std::vector<double> &y)
{
  int total_count = 0;
  std::ifstream ifs_data(_file_data);

  data.clear();

  while (!ifs_data.eof())
  {
    points tmp;
    std::string line;
    std::getline(ifs_data, line);
    if (line == "") break;
    std::stringstream read_line(line);
    /* implement here read_line */
    data.push_back(tmp);
    ++total_count;
  }

  ifs_data.close();

  y.clear();
  y.resize(total_count);

  int count = 0;
  std::ifstream ifs_y(_file_y);

  while (!ifs_y.eof())
  {
    if (count >= total_count) break;
    double tmp;
    std::string line;
    std::getline(ifs_y, line);
    tmp = std::stod(line);
    y[count] = tmp;
    ++count;
  }

  ifs_y.close();
};

// @brief Split data for cross validation
template <typename T>
void SplitValidation(std::vector<T> _orig, std::vector<T> &_learn,
		     std::vector<T> &_test, int _seed, int _elements)
{
  if (_elements >= _orig.size())
    _elements = 1;

  std::vector<int> selection(_elements);

  // Choose the data to leave out
  for (unsigned int i = 0; i < _elements; ++i)
  {
    int selection_candidate = (i + 1) * _seed;
    // If candidate iterator does not exist
    if (selection_candidate >= _orig.size())
    {
      selection_candidate %= _orig.size();
      auto iter = std::find(selection.begin(), selection.end(),
			    selection_candidate);
      // While candidate iterator duplicates
      while (iter != selection.end())
      {
	++selection_candidate;
	iter = std::find(selection.begin(), selection.end(),
			 selection_candidate);
      }
    }
    selection[i] = selection_candidate;
  }
  std::sort(selection.begin(), selection.end());

  _test.resize(_elements);
  _learn.resize(_orig.size() - _elements);

  for (unsigned int i = 0; i < _elements; ++i)
    _test[i] = _orig[selection[i]];

  unsigned int j = 0;
  unsigned int k = 0;
  for (unsigned int i = 0; i < _orig.size(); ++i)
  {
    while (i == selection[k])
    {
      ++i;
      ++k;
      if (k >= selection.size()) break;
    }
    if (i >= _orig.size()) break;
    
    _learn[j] = _orig[i];
    ++j;
  }
};

// @brief Learn calibration model through cross validation
void CreateModel(std::string data_file, std::string y_file,
		 std::string save_file)
{
  std::vector<points> data_learn_orig;
  std::vector<double> y_learn_orig;

  std::vector<points> data_learn;
  std::vector<double> y_learn;
  std::vector<points> data_test;
  std::vector<double> y_validation;

  std::cout << "Read...\n";

  Read(data_file, y_file, data_learn_orig, y_learn_orig);

  svm_parameter param;
  svm_model model;

  std::vector<double> epsilon = {0, 0.01, 0.1, 0.5, 1, 2, 4};
  std::vector<double> Cs(cross_validation);
  std::vector<double> gammas(cross_validation);
  std::vector<double> epsilons(cross_validation);
  std::vector<std::vector<double> > accuracies(cross_validation);

  for (unsigned int n = 0; n < cross_validation; ++n)
  {
    data_learn.clear();
    data_test.clear();
    y_learn.clear();
    y_validation.clear();

    SplitValidation<points>(data_learn_orig, data_learn, data_test,
			    n + validation_seed, leave_n_out);
    SplitValidation<double>(y_learn_orig, y_learn, y_validation,
			    n + validation_seed, leave_n_out);

    double best_C, best_gamma, best_epsilon;
    std::vector<double> best_accuracy_vec(y_validation.size() + 1);
    double best_accuracy = 100 * y_validation.size(); // should not be worse than this

    for (unsigned int i = 0; i < 20; ++i)
      for (unsigned int j = 0; j < 20; ++j)
	for (unsigned int k = 0; k < epsilon.size(); ++k)
	  {
	    double now_C = std::pow(2, static_cast<int>(i - 5));
	    double now_gamma = std::pow(2, static_cast<int>(j - 15));
	    std::cout << "trying... C = " << now_C
		      << " gamma = " << now_gamma
		      << " epsilon = " << epsilon[k] << "\n";
	    SetParameters(param, now_C, now_gamma, epsilon[k]);
	    std::vector<double> accuracy_candidate =
	      SVR(&model, param, data_learn, y_learn,
		  data_test, y_validation);
	    if (accuracy_candidate[0] < best_accuracy)
	    {
	      best_accuracy = accuracy_candidate[0];
	      best_accuracy_vec = accuracy_candidate;
	      best_C = now_C;
	      best_gamma = now_gamma;
	      best_epsilon = epsilon[k];
	    }
	    std::cout << "\n";
	  }

    Cs[n] = best_C;
    gammas[n] = best_gamma;
    epsilons[n] = best_epsilon;
    accuracies[n] = best_accuracy_vec;
  }

  // Choost the best parameter out of the best parameters in each validation
  double best_C, best_gamma, best_epsilon;
  std::vector<std::vector<double> > best_accuracies;
  double minimum_error = 100 * y_validation.size() * cross_validation;
  for (unsigned int i = 0; i < cross_validation; ++i)
  {
    for (unsigned int j = 0; j < y_validation.size(); ++j)
    {
      if (accuracies[i][j + 1] < 5.0)
	std::cout << "error is "
		  << accuracies[i][j + 1] << "\n";
      else
	std::cout << "\033[1;31merror is "
		  << accuracies[i][j + 1] << "\033[0m\n";
    }

    std::cout << "Result...\n"
	      << "C = " << Cs[i]
	      << " gamma = " << gammas[i]
	      << " epsilon = " << epsilons[i] << "\n\n";

    SetParameters(param, Cs[i], gammas[i], epsilons[i]);

    double total_err = 0.0;
    std::vector<std::vector<double> > accuracies_candidate(cross_validation);
    for (unsigned int n = 0; n < cross_validation; ++n)
    {
      data_learn.clear();
      data_test.clear();
      y_learn.clear();
      y_validation.clear();

      SplitValidation<points>(data_learn_orig, data_learn, data_test,
			      n + validation_seed, leave_n_out);
      SplitValidation<double>(y_learn_orig, y_learn, y_validation,
			      n + validation_seed, leave_n_out);

      std::vector<double> err =
	SVR(&model, param, data_learn, y_learn, data_test, y_validation);
      total_err += err[0];
      accuracies_candidate[n] = err;
    }

    if (total_err < minimum_error)
    {
      best_C = Cs[i];
      best_gamma = gammas[i];
      best_epsilon = epsilons[i];
      best_accuracies = accuracies_candidate;
    }
  }

  std::cout << "FinalResult...\n";
  for (unsigned int i = 0; i < cross_validation; ++i)
    for (unsigned int j = 0; j < y_validation.size(); ++j)
    {
      if (best_accuracies[i][j + 1] < 5.0)
	std::cout << "error is "
		  << best_accuracies[i][j + 1] << "\n";
      else
	std::cout << "\033[1;31merror is "
		  << best_accuracies[i][j + 1] << "\033[0m\n";
    }

  std::cout << "\nC = " << best_C
	    << " gamma = " << best_gamma
	    << " epsilon = " << best_epsilon << "\n\n";

  SetParameters(param, best_C, best_gamma, best_epsilon);
  SaveModel(save_file, &model, param, data_learn_orig, y_learn_orig);
};


int main(int argc, char **argv)
{
  if (argc != 3) return -1;

  CreateModel(argv[0], argv[1], argv[2]);

  return 0;
}
