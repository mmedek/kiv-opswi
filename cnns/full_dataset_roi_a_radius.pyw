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

path1 = 'C:\\Users\\mmedek.MMEDEK-NB\\Documents\\Python Scripts\\data_roi_a_radius'    #path of folder of images    
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
label[0:575]=1
label[575:]=0
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
nb_epoch = 10


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

X_train1 = X[0:500]
X_train2 = X[575:1075]
X_train = np.vstack((X_train1, X_train2))

X_test1 = X[500:575]
X_test2 = X[1075:]
X_test = np.vstack((X_test1, X_test2))

y_test = np.empty((150, 1), dtype=int)
a = 75
for i in range(a):
    y_test[i] = 1

for i in range(a, 150):
    y_test[i] = 0

   
y_train = np.empty((1000, 1), dtype=int)
a = 500
for i in range(a):
    y_train[i] = 1 
    
for i in range(a,1000):
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


# serialize model to JSON
model_json = model.to_json()
with open("model.json", "w") as json_file:
    json_file.write(model_json)
# serialize weights to HDF5
model.save_weights("model.h5")
print("Saved model to disk")

#%% 
results = model.predict_classes(X_test[0:150]);
print(results)
#%% 

# input image
original_image = cv2.imread('C:\\Users\\mmedek.MMEDEK-NB\\Documents\\Visual Studio 2015\\Projects\\ROISelector\\ROISelector\\cnn\\orig_neg_train.jpg', 0)

# equalize input image
equalized_image = cv2.equalizeHist(original_image)
# image size same as in traninig
img_rows, img_cols = 128, 128


# for i in range(0, len(self.data)):
# positive ROI
template = cv2.imread('C:\\Users\\mmedek.MMEDEK-NB\\Documents\\Visual Studio 2015\\Projects\\ROISelector\\ROISelector\\cnn\\roi.jpg', 0)


w, h = template.shape[::-1]
 
original_image = equalized_image.copy()
method = eval('cv2.TM_CCOEFF_NORMED')


# Apply template Matching
res = cv2.matchTemplate(original_image,template,method)
min_val, max_val, min_loc, max_loc = cv2.minMaxLoc(res)

# If the method is TM_SQDIFF or TM_SQDIFF_NORMED, take minimum
if method in [cv2.TM_SQDIFF, cv2.TM_SQDIFF_NORMED]:
    top_left = min_loc
else:
    top_left = max_loc

cropped_image = original_image[top_left[1] : top_left[1] + h, top_left[0] : top_left[0] + w]
cropped_image = cropped_image.astype('float32')
cropped_image /= 255
cv2.imwrite('C:\\Users\\mmedek.MMEDEK-NB\\Documents\\Visual Studio 2015\\Projects\\ROISelector\\ROISelector\\cnn\\cropped_image.jpg', cropped_image)




X_test = cropped_image; 
X_test = np.expand_dims(X_test, axis=0)
X_test = np.expand_dims(X_test, axis=0)
print(X_test.shape)
results = model.predict_classes(X_test);
print(results)

