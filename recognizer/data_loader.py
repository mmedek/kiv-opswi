# -*- coding: utf-8 -*-
"""
Created on Mon Mar 27 22:17:42 2017

@author: mmedek
"""
import os
import sys
import evaluation_data


class DataLoader:
    def __init__(self, input):
        "e.g. 'BlueTooth'"
        self.component = input

    def __find_folder(self):
        "expected data with cnns in folder 'data'"
        list_folders = os.listdir(os.curdir + "/data")
        match = next((x for x in list_folders if x == self.component), None)

        if match == None:
            sys.exit('Folder \'' + self.component + '\' not found!')
        else:
            return match

    def get_data(self):
        "find folder which folders with cnn according to component name e.g. 'BlueTooth'"
        folder = self.__find_folder()
        list_folders = os.listdir(os.curdir + "/data" + "/" + folder)

        data_list = []
        for x in list_folders:
            curr_items = [os.curdir + "/data" + "/" + folder + "/" + x];
            curr_items.extend(os.listdir(os.curdir + "/data" + "/" + folder + "/" + x))
            data_list.append(evaluation_data.EvaluationData(curr_items))
            "print curr folder"
            print('|-- ' + curr_items[0])
            "print content of folder"
            for y in range(1, len(curr_items)):
                print('   |-- ' + curr_items[y])

        print('Data from folder structure was successfully loaded')
        return data_list
