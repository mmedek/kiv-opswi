# -*- coding: utf-8 -*-
"""
Created on Tue Mar 28 21:52:47 2017

@author: mmedek
"""
import cv2
import error
import numpy as np
import os

from keras.models import model_from_json


class CNNRunner:
    CONST_WIDTH_LINE = 15
    ERRORS_IMG_PATH = "error.jpg"

    def __init__(self, data, image_path):
        self.data = data
        self.image_path = image_path

    def apply_pattern_match(self):

        # input image
        original_image = cv2.imread(self.image_path, 0)
        # equalize input image
        equalized_image = cv2.equalizeHist(original_image)
        err = False
        list_err = []

        for i in range(0, len(self.data)):
            # positive ROI
            template = cv2.imread(self.data[i].image_path, 0)
            w, h = template.shape[::-1]

            original_image = equalized_image.copy()
            method = eval('cv2.TM_CCOEFF_NORMED')

            # Apply template Matching
            res = cv2.matchTemplate(original_image, template, method)
            min_val, max_val, min_loc, max_loc = cv2.minMaxLoc(res)

            # If the method is TM_SQDIFF or TM_SQDIFF_NORMED, take minimum
            if method in [cv2.TM_SQDIFF, cv2.TM_SQDIFF_NORMED]:
                top_left = min_loc
            else:
                top_left = max_loc

            cropped_image = original_image[top_left[1]: top_left[1] + h, top_left[0]: top_left[0] + w]
            cropped_image = cropped_image.astype('float32')
            cropped_image /= 255

            # load json and create model
            json_file = open(self.data[i].model_json, 'r')
            loaded_model_json = json_file.read()
            json_file.close()
            loaded_model = model_from_json(loaded_model_json)
            loaded_model.compile(loss='binary_crossentropy', optimizer='adadelta', metrics=["accuracy"])
            # load weights into new model
            loaded_model.load_weights(self.data[i].model_h5)

            X_test = cropped_image;
            X_test = np.expand_dims(X_test, axis=0)
            X_test = np.expand_dims(X_test, axis=0)
            results = loaded_model.predict_classes(X_test, verbose=0);

            if results[0] == 1:
                err = True
                err_name = os.path.normpath(self.data[i].folder)
                err_name.split(os.sep)
                list_err.append(error.Error(int(top_left[0] + w / 2), int(top_left[1] + h / 2), err_name[len(err_name) - 1]))

        print('Computing finished')

        if err == True:
            colorful_image = cv2.cvtColor(original_image, cv2.COLOR_GRAY2RGB)
            for i in range(0, len(list_err)):
                cv2.line(colorful_image, (list_err[i].x - CNNRunner.CONST_WIDTH_LINE, list_err[i].y),
                         (list_err[i].x + CNNRunner.CONST_WIDTH_LINE, list_err[i].y), (0, 0, 255), 3)
                cv2.line(colorful_image, (list_err[i].x, list_err[i].y - CNNRunner.CONST_WIDTH_LINE),
                         (list_err[i].x, list_err[i].y + CNNRunner.CONST_WIDTH_LINE), (0, 0, 255), 3)
                cv2.circle(colorful_image, (list_err[i].x, list_err[i].y), int(CNNRunner.CONST_WIDTH_LINE / 2),
                           (0, 0, 255), -1)
                font = cv2.FONT_HERSHEY_SIMPLEX
                cv2.putText(colorful_image, list_err[i].err_name, (int(list_err[i].x + CNNRunner.CONST_WIDTH_LINE / 2),
                                                                   int(list_err[i].y - CNNRunner.CONST_WIDTH_LINE / 2)),
                            font, 1, (0, 0, 255), 2)
            print('Errors were found. Saving image with errors to ' + CNNRunner.ERRORS_IMG_PATH)
            cv2.imwrite(CNNRunner.ERRORS_IMG_PATH, colorful_image)
        else:
            print('No errors found')

