# -*- coding: utf-8 -*-
"""
Created on Mon Mar 27 22:13:20 2017

@author: mmedek
"""

import record
import data_loader
import cnn_runner
import sys

if len(sys.argv) != 2:
    print("Expected one input argument. Run script: python run.py [image path]")
else:
    im_record = record.Record(sys.argv[1])
    data_loader = data_loader.DataLoader(im_record.component)
    list_folders = data_loader.get_data()
    cnn_runner = cnn_runner.CNNRunner(list_folders, im_record.full_path)
    cnn_runner.apply_pattern_match()