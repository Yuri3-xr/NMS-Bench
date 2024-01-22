import os
method = str(input())

input_file = 'data/nms_yolov8n/'
output_file = 'data/nms_yolov8n_output' + '_' + method + '/' 


command = './build/output/main' + ' ' + input_file + ' ' + output_file  + ' ' + method
os.system(command)
print(command)

print("[done!]")