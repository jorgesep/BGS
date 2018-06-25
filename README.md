# Software for evaluating BGS algorithms

This Repository contains the implementation of some background subtraction methods, based on statistical modeling of pixels behavior within an image sequence. The background behavior of an image sequence can be modeled either with a mixture of Gaussian distributions [1][2] or Gaussian Kernels. The code is based on a set of C++ classes along with the OpenCV library. Additionally, a binary program is available to evaluate the results of background subtraction algorithms included in this repository; the foreground masks generated by the methods can be compared against a ground truth (reference frame). The program compares pixel-to-pixel the produced silhouettes with its ground truth, resulting in a group of True/False Positive/Negative metrics which are combined to produce a general F1 score measure.
