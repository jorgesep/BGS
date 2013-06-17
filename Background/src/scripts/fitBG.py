import numpy as np
from scipy.optimize import leastsq
import matplotlib.pyplot as plt
from pylab import *
import sys

def norm(x, mean, sd,maxi):
  norm = []
  for i in arange(x.size):
    norm += [maxi*1/(sd*np.sqrt(2*np.pi))*np.exp(-(x[i] - mean)**2/(2*sd**2))]
  return np.array(norm)
  
mean1 = 215
std1 =  10 
maxi = 90

#reading data

#data_r = loadtxt('point_300_300.txt',dtype='float')
#data_r = loadtxt('pnof_680_200.txt',dtype='float')
data_r = loadtxt('pwf_680_200.txt',dtype='float')
DATA   =  data_r.transpose()
#Y = DATA[21]
#Y = DATA[7]
Y = DATA[2]
  
#plotting the histogram
a,b,c = hist(Y,100)
#a,b,c = hist(Y,30)
#a,b,c = hist(Y,10)

x      = b[:-1]
y_real = a 

######################################
# Solving
m, dm, sd1,maxi1 = [147, 4, 20,300]
p = [m, dm, sd1,maxi1] # Initial guesses for leastsq
y_init = norm(x, m, sd1,maxi1) # For final comparison plot

def res(p, y, x):
  m, dm, sd1,maxi1 = p
  m1 = m
  m2 = m1 + dm
  y_fit = norm(x, m1, sd1,maxi1) 
  err = y - y_fit
  return err

plsq = leastsq(res, p, args = (y_real, x))

X = arange(120,180,0.1)
Y_est = norm(X, plsq[0][0], plsq[0][2],plsq[0][3]) 

y_est = norm(x, plsq[0][0], plsq[0][2],plsq[0][3]) 
print '%5s %5s %5s' %('Xo','Std','Max' )
print '%2.3f %2.3f %2.3f' %(plsq[0][0], plsq[0][2],plsq[0][3])
#xTextPos = (min(x))+0.05*(min(x))
#yTextPos = (max(y_real))-0.1*(max(y_real))

plt.plot(x, y_real, label='Real Data')
plt.plot(x, y_est, '-r' ,label='Fitted')      
plt.plot(X, Y_est, '-r' ,label='Fitted')      
#plt.xlim((150,255))
show()
