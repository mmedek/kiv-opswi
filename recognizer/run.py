# -*- coding: utf-8 -*-
"""
Created on Mon Mar 27 22:13:20 2017

@author: mmedek
"""

import record
import data_loader
import cnn_runner

im_record = record.Record("ABoard_TX-55AS650B_%NH-4540005(31511).jpg")
data_loader = data_loader.DataLoader(im_record.component)
list_folders = data_loader.get_data()
cnn_runner = cnn_runner.CNNRunner(list_folders, im_record.full_path);
cnn_runner.apply_pattern_match()