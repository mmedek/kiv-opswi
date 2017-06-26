# -*- coding: utf-8 -*-
"""
Created on Tue Mar 28 08:21:22 2017

@author: mmedek
"""
import sys


class EvaluationData:
    folder = ""
    image_path = ""
    model_h5 = ""
    model_json = ""

    def __init__(self, input_field):

        if (len(input_field) != 4):
            sys.exit('In Folder \'' + input_field[0] + '\' is not three items (image.jpg, model.h5, model.json)!')

        self.folder = input_field[0]
        # model and image can be sorted differently according to filesystem!
        if (input_field[1].rsplit('.', 1)[1] != EvaluationData.__CONST_H5(self)):
            sys.exit('In Folder \'' + input_field[0] + '\' was not found image.jpg')
        self.model_h5 = self.folder + '/' + input_field[1]

        if (input_field[2].rsplit('.', 1)[1] != EvaluationData.__CONST_JPG(self)):
            sys.exit('In Folder \'' + input_field[0] + '\' was not found h5 model file')
        self.image_path = self.folder + '/' + input_field[2]

        if (input_field[3].rsplit('.', 1)[1] != EvaluationData.__CONST_JSON(self)):
            sys.exit('In Folder \'' + input_field[0] + '\' was not found json model file')
        self.model_json = self.folder + '/' + input_field[3]

    def __CONST_JPG(self):
        return "jpg"

    def __CONST_H5(self):
        return "h5"

    def __CONST_JSON(self):
        return "json"