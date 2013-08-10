#!/usr/bin/python



import pylab as pl
from math import *

class analyse_nonparametric :

    def __init__(self):
        print 'Reading Non-parametric'
        self.colorList={'1':(0.98,0.01,0.74),\
                '2' : (1.0,0.60,0.00),\
                '3' : 'r',\
                '4' : (0.62,0.39,0.27),\
                '5' : (0.14,0.96,0.05),\
                '6' : 'g',\
                '7' : 'c',\
                '8' : (0.41,0.10,0.51),\
                '9' : 'b',\
                '10': (0.20,0.20,0.20)}

    def load(self, name) :
        Fi = file(name,'r')

        lines = Fi.read().split('\n')

        self.parameters = {lines[0].split()[i+1]:i for i in range(len(lines[0].split()[1:])) if lines[0].split()[0] == '#'}
        self.data  = [l0.split() for l0 in lines[1:] if len(l0.split()) > 1]

        self.tp_idx = self.parameters['TPR_MEAN']
        self.fp_idx = self.parameters['FPR_MEAN']
        self.th_idx = self.parameters['Threshold']
        self.ap_idx = self.parameters['Alpha']

    def _generic_plot(self,col1,col2):
        '''col1: is name of the parametric curve
           col2: is variable parameter
        '''

        index1 = self.parameters[col1]
        index2 = self.parameters[col2]
        fpr    = self.fp_idx
        tpr    = self.tp_idx

        print "INDEX: " + str(index1) + " " + str(index2) + " " + str(fpr) +  " " + str(tpr)

        pl.clf()
        pl.subplot(111)

        params = {p0[index1]:p0[index1] for p0 in self.data}
        values = [float(val) for val in params.keys()]
        values.sort()
        params=values


        i=0
        self.legendP=[]
        self.legendT=[]

        # arranged by index1.
        for key in params :

            print "key: " + str(key)
            if key == 2e-09 or key == 0.0001:
                # get subset of data indexed by 'key', key is index1. 
                _data = [p0 for p0 in self.data if float(p0[index1]) == key]

                # get dictionary of same data taking 'index2' as key
                _thre = {float(p0[index2]):p0 for p0 in _data}
                values = _thre.keys()
                values.sort()

                # create a list parameters in ascendent ordered by index2 (col2)
                X = [float(_thre[val][fpr])    for val in values]
                Y = [float(_thre[val][tpr])    for val in values]
                T = [float(_thre[val][index2]) for val in values]
                A = [float(_thre[val][index1]) for val in values]
          
                p=int(fmod(i,len(self.colorList)))



                self.legendP.append(pl.plot(X,Y,color=self.colorList[str(p+1)])[0])

                # preparation of legend box in plot.
                legend = self.parameters.keys()[self.parameters.values().index(index1)]
                self.legendT.append(str(legend) + '=' + str(key))
                #legendT.append('Alpha='+str(key))
                #if r == 30 :
                pl.plot(X,Y,color=self.colorList[str(p+1)],marker='x')
 
                for i in range(len(X)):
                    if Y[i] > 0.8 :
                        if A[i] == 0.001 :
                            pl.text(X[i]+0.0000,Y[i]+0.02,T[i],fontsize=11,color='black', horizontalalignment='center',verticalalignment='center')
                        else:
                            pl.text(X[i]+0.00025,Y[i]+0.02,T[i],fontsize=11,color='black', horizontalalignment='center',verticalalignment='center')
                    else:
                        pl.text(X[i]+0.00025,Y[i],T[i],fontsize=11,color='black', horizontalalignment='center',verticalalignment='center')

                i+=1
                pl.savefig('NP_TPR-FPR4.png')
                #return
        #
        pl.legend(self.legendP,self.legendT, bbox_to_anchor=(0.99,0.56), numpoints=1)
        pl.title('Non Parametric Model\nMuHAVI ROC Curve')
        pl.xlabel('False Positive Rate')
        pl.ylabel('True Positive Rate')
        pl.grid()
        #
        pl.savefig('NP_TPR-FPR4.png')
        
         

    def plot1(self):
        '''Plot mean'''
        pl.clf()
        pl.subplot(111)

        tpr_index = self.parameters['TPR_MEAN']
        fpr_index = self.parameters['FPR_MEAN']

        print tpr_index
        print fpr_index

        X=[]
        Y=[]

        for p0 in self.data :
            #print p0
            X.append(float(p0[fpr_index]))
            Y.append(float(p0[tpr_index]))
        
        #
        pl.plot(X,Y,'b-',lw=1)
        pl.title('Non Parametric\nMuHAVI ROC Curve')
        pl.xlabel('False Positive Rate')
        pl.ylabel('True Positive Rate')
        pl.grid()
        #
        pl.savefig('NP_TPR-FPR.png')

    def plot2(self):
        '''Threshold'''

        pl.clf()
        pl.subplot(111)

        tpr_index = self.parameters['TPR_MEAN']
        fpr_index = self.parameters['FPR_MEAN']
        thr_index = self.parameters['Threshold']
        alp_index = self.parameters['Alpha']

        print thr_index

        param = {p0[alp_index]:p0[alp_index] for p0 in self.data}
        values= [float(val) for val in param]
 
        values.sort()
        param=values
        print param
        colorList={'1':(0.98,0.01,0.74),'3':'r','2':(1.0,0.60,0.00),'4':(0.62,0.39,0.27),'5':(0.14,0.96,0.05),'6':'g','7':'c','9':'b','8':(0.41,0.10,0.51),'10':(0.20,0.20,0.20)}
        i=0
        legendP=[]
        legendT=[]

        for t in param :

            X=[float(p0[fpr_index]) for p0 in self.data if float(p0[alp_index]) == t]
            Y=[float(p0[tpr_index]) for p0 in self.data if float(p0[alp_index]) == t]
            T=[float(p0[thr_index]) for p0 in self.data if float(p0[alp_index]) == t]
            A=[float(p0[alp_index]) for p0 in self.data if float(p0[alp_index]) == t]
           
            p=int(fmod(i,len(colorList)))
            print p
            legendP.append(pl.plot(X,Y,color=colorList[str(p+1)])[0])
            legendT.append('Threshold='+str(t))
            #if r == 30 :
            pl.plot(X,Y,color=colorList[str(p+1)],marker='x')                
 
            i+=1
        #
        #pl.legend(legendP,legendT, bbox_to_anchor=(0.95,0.76), numpoints=1)
        pl.title('Non Parametric Model\nMuHAVI ROC Curve')
        pl.xlabel('False Positive Rate')
        pl.ylabel('True Positive Rate')
        pl.grid()
        #
        pl.savefig('NP_TPR-FPR2.png')
        
    def plot3(self):
        '''Alpha'''

        index     = self.ap_idx
        threshold = self.th_idx

        pl.clf()
        pl.subplot(111)

        params = {p0[index]:p0[index] for p0 in self.data}
        values= [float(val) for val in params]
        values.sort()
        params=values

        i=0
        legendP=[]
        legendT=[]

        #ordered by alpha
        for key in params :

            # get subset of data indexed by 'key'. In this time 'key' is alpha 
            _data = [p0 for p0 in self.data if float(p0[index]) == key]

            # get dictionary of same data taking 'threshold' as key
            _thre = {float(p0[threshold]):p0 for p0 in _data}
            values = _thre.keys()
            values.sort()

            # create a list parameters in ascendent order by threshold
            X = [float(_thre[val][self.fp_idx]) for val in values]
            Y = [float(_thre[val][self.tp_idx]) for val in values]
            T = [float(_thre[val][self.th_idx]) for val in values]
            A = [float(_thre[val][self.ap_idx]) for val in values]
          
            p=int(fmod(i,len(self.colorList)))

            legendP.append(pl.plot(X,Y,color=self.colorList[str(p+1)])[0])

            # preparation of legend box in plot.
            legend = self.parameters.keys()[self.parameters.values().index(index)]
            legendT.append(str(legend) + '=' + str(key))
            #legendT.append('Alpha='+str(key))
            #if r == 30 :
            pl.plot(X,Y,color=self.colorList[str(p+1)],marker='x')                
 
            i+=1
        #
        pl.legend(legendP,legendT, bbox_to_anchor=(0.99,0.56), numpoints=1)
        pl.title('Non Parametric Model\nMuHAVI ROC Curve')
        pl.xlabel('False Positive Rate')
        pl.ylabel('True Positive Rate')
        pl.grid()
        #
        pl.savefig('NP_TPR-FPR3.png')
        
    def plot4(self):

        self._generic_plot('Threshold','Alpha')
        #pl.legend(self.legendP,self.legendT, bbox_to_anchor=(0.99,0.56), numpoints=1)
        #pl.title('Non Parametric Model\nMuHAVI ROC Curve')
        #pl.xlabel('False Positive Rate')
        #pl.ylabel('True Positive Rate')
        #pl.grid()
        ##
        #pl.savefig('NP_TPR-FPR4.png')
        
    

class sepu :

    # ########################
    #
    def  __init__(self):
        # define parameters
        print 'sepu'
        
    # ######################
    #
    def load(self,name) :
        Fi=file(name,'r')  
        
        l=Fi.read().split('\n')
        self.data=[]
        for l0 in l :
            l1=l0.split()
            if len(l1) > 1 and l1[0] != '#':
                self.data.append(l1)
    # ######################
    #
    def plot1(self) :
                
        
        pl.clf()
        pl.subplot(111) 
        
        X=[]
        Y=[]
        for p0 in self.data :
            #print p0
            X.append(float(p0[10]))
            Y.append(float(p0[9]))
        
        pl.plot(X,Y,'o')

            

        #
        pl.xlabel('TPR')
        pl.ylabel('FPR')
        #
        pl.savefig('TPR-FPR.png')


    # ######################
    #
    def plot2(self) :
                
        
        pl.clf()
        pl.subplot(111) 
        
        X=[]
        Y=[]
        for p0 in self.data :
            
            if float(p0[9]) > 0.4 :
                X.append(float(p0[10]))
                Y.append(float(p0[9]))
        
                
        pl.plot(X,Y,'o')

        

        #
        pl.xlabel('FPR')
        pl.ylabel('TPR')
        #
        pl.savefig('TPR-FPR2.png')



    # ######################
    #
    def plot3(self) :
                
        
        pl.clf()
        pl.subplot(111) 
        
        X=[]
        Y=[]
        R=[]
        for p0 in self.data :            
            if float(p0[9]) > 0.4 :
                X.append(float(p0[10]))
                Y.append(float(p0[9]))
                R.append(p0[3])
        
                
        pl.plot(X,Y,'o')
        
        for i in range(len(X)): 
            pl.text(X[i],Y[i]-0.006,R[i],fontsize=11,color='black', horizontalalignment='center',verticalalignment='center')

            
        #
        pl.xlabel('FPR')
        pl.ylabel('TPR')
        #
        pl.savefig('TPR-FPR3.png')


    # ######################
    #
    def plot4(self) :
                
        
        pl.clf()
        pl.subplot(111) 
        


        rang={}
        for p0 in self.data :            
            rang[p0[3]]=p0[3]
        rang2=[]
        for r in rang:
            rang2.append(float(r))
        
        print rang2
        rang2.sort()
        rang=rang2
        print rang
        colorList={'1':(0.98,0.01,0.74),'3':'r','2':(1.0,0.60,0.00),'4':(0.62,0.39,0.27),'5':(0.14,0.96,0.05),'6':'g','7':'c','9':'b','8':(0.41,0.10,0.51),'10':(0.20,0.20,0.20)}
        i=0
        legendP=[]
        legendT=[]
        for r in rang :
            
            X=[]
            Y=[]
            R=[]
            A=[]
            
            for p0 in self.data :            
                if float(p0[3]) == r :
                    X.append(float(p0[10]))
                    Y.append(float(p0[9]))
                    R.append(float(p0[3]))
                    A.append(float(p0[0]))
            
            p=int(fmod(i,len(colorList)))
            print p
            legendP.append(pl.plot(X,Y,color=colorList[str(p+1)])[0])
            legendT.append('Range='+str(r))
            #if r == 30 :
            pl.plot(X,Y,color=colorList[str(p+1)],marker='x')                
            if r == 10 :
                pl.plot(X,Y,color=colorList[str(p+1)],marker='x')                
                for i in range(len(X)):
                    if Y[i] > 0.8 :
                        if A[i] == 0.001 :
                            pl.text(X[i]+0.0000,Y[i]+0.02,A[i],fontsize=11,color='black', horizontalalignment='center',verticalalignment='center')
                        else:
                            pl.text(X[i]+0.00025,Y[i]+0.02,A[i],fontsize=11,color='black', horizontalalignment='center',verticalalignment='center')
                    else:
                        pl.text(X[i]+0.00025,Y[i],A[i],fontsize=11,color='black', horizontalalignment='center',verticalalignment='center')
            i+=1
        #
        pl.legend(legendP,legendT, bbox_to_anchor=(0.95,0.76), numpoints=1)
        pl.title('Self Adaptive Gaussian Mixture Model\nMuHAVI ROC Curve')
        pl.xlabel('False Positive Rate')
        pl.ylabel('True Positive Rate')
        pl.grid()
        #
        pl.savefig('TPR-FPR4.png')
        
    






#############
        
if __name__ == '__main__':
 
    ##########
    #Variables
    #########
    s=sepu()

#  Alpha=1 cf=2 bgRation=3 Range=4 Ge=5n GaussiansNo=6 Sigma=7 cT=8 Tau=9 TPR=10 FPR=11 SPE MCC TPR TNR SPE MCC

    s.load('final_measures.txt')
    
    s.plot1()
    s.plot2()
    s.plot3()
    s.plot4()
                       
    n = analyse_nonparametric()
    n.load('np_final_measures.txt')
    n.plot1()
    n.plot2()
    n.plot3()
    n.plot4()


