#KERAS
from keras.models import Sequential
from keras.layers.core import Dense, Dropout, Activation, Flatten
from keras.layers.convolutional import Convolution2D, MaxPooling2D
from keras.optimizers import SGD,RMSprop,adam
from keras.utils import np_utils

from keras import backend as K
K.set_image_dim_ordering('th')

import numpy as np
import matplotlib.pyplot as plt
import matplotlib
import os
import theano
from PIL import Image
from numpy import *
# SKLEARN
from sklearn.utils import shuffle
from sklearn.cross_validation import train_test_split

# input image dimensions
img_rows, img_cols = 128, 128

# number of channels
img_channels = 1

#%%
#  data

path1 = 'C:\\Users\\mmedek.MMEDEK-NB\\Documents\\Python Scripts\\data_roi_b'    #path of folder of images    
path2 = 'C:\\Users\\mmedek.MMEDEK-NB\\Documents\\Python Scripts\\data_gray'  #path of folder to save images    

listing = os.listdir(path1) 
num_samples=len(listing)

for file in listing:
    im = Image.open(path1 + '\\' + file)   
    img = im.resize((img_rows,img_cols))
    gray = img.convert('L')
                #need to do some more processing here           
    gray.save(path2 +'\\' +  file, "JPEG")

imlist = os.listdir(path2)

im1 = array(Image.open(path2 + '\\'+ imlist[0])) # open one image to get size
m,n = im1.shape[0:2] # get the size of the images
imnbr = len(imlist) # get the number of images

# create matrix to store all flattened images
immatrix = array([array(Image.open(path2 + '\\' + im2)).flatten()
              for im2 in imlist],'f')
            
label=np.ones((num_samples,),dtype = int)
label[0:15]=1
label[15:]=0
#%%
#data,Label = shuffle(immatrix,label, random_state=2)
data,Label = immatrix,label
train_data = [data,Label]

#%%

#batch_size to train
batch_size = 8
# number of output classes
nb_classes = 2
# number of epochs to train
nb_epoch = 25


# number of convolutional filters to use
nb_filters = 32
# size of pooling area for max pooling
nb_pool = 2
# convolution kernel size
nb_conv = 3

#%%
(X, y) = (train_data[0],train_data[1])


# STEP 1: split X and y into training and testing sets

#X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.9685)

X_train1 = X[0:12]
X_train2 = X[15:27]
X_train = np.vstack((X_train1, X_train2))

X_test1 = X[12:15]
X_test2 = X[27:750]
X_test = np.vstack((X_test1, X_test2))

y_test = np.empty((726, 1), dtype=int)
a = 3
for i in range(a):
    y_test[i] = 1

a = 726
for i in range(3, a):
    y_test[i] = 0

   
y_train = np.empty((24, 1), dtype=int)
a = 12
for i in range(a):
    y_train[i] = 1 
    
for i in range(a,24):
    y_train[i] = 0 

#%%
X_train = X_train.reshape(X_train.shape[0], 1, img_rows, img_cols)
X_test = X_test.reshape(X_test.shape[0], 1, img_rows, img_cols)

X_train = X_train.astype('float32')
X_test = X_test.astype('float32')

X_train /= 255
X_test /= 255

print('X_train shape:', X_train.shape)
print(X_train.shape[0], 'train samples')
print(X_test.shape[0], 'test samples')

# convert class vectors to binary class matrices
Y_train = np_utils.to_categorical(y_train, nb_classes)
Y_test = np_utils.to_categorical(y_test, nb_classes)

i = 10
print("label : ", Y_train[i,:])

#%%

model = Sequential()

model.add(Convolution2D(nb_filters, nb_conv, nb_conv,
                        border_mode='valid',
                        input_shape=(1, img_rows, img_cols)))
convout1 = Activation('relu')
model.add(convout1)
model.add(Convolution2D(nb_filters, nb_conv, nb_conv))
convout2 = Activation('relu')
model.add(convout2)
model.add(MaxPooling2D(pool_size=(nb_pool, nb_pool)))
model.add(Dropout(0.5))

model.add(Flatten())
model.add(Dense(128))
model.add(Activation('relu'))
model.add(Dropout(0.5))
model.add(Dense(nb_classes))
model.add(Activation('softmax'))
model.compile(loss='binary_crossentropy', optimizer='adadelta', metrics=["accuracy"])
# print(model.summary())
#%%

hist = model.fit(X_train, Y_train, batch_size=batch_size, nb_epoch=nb_epoch,
              verbose=1, validation_data=(X_test, Y_test))
            
            
#hist = model.fit(X_train, Y_train, batch_size=batch_size, nb_epoch=nb_epoch,
 #             verbose=1, validation_split=0.969)


#%%     

score = model.evaluate(X_test, Y_test, verbose=1)
print('Test lost:', score[0])
print('Test accuracy:', score[1])

#%% 
results = model.predict_classes(X_test[0:723]);
print(model.predict_classes(X_test[0:723]))
print(Y_test[0:723])