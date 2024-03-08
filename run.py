import os

method = str(input())

input_file = '/data02/sl/nms_data/nms_yolov8n_crowdhuman_gpu/'
# gpu / cpu dataset is a little different, pay attention
output_file = 'result/nms_yolov8n_crowdhuman_output' + '_' + method + '/' 


command = './build/output/main' + ' ' + input_file + ' ' + output_file  + ' ' + method
os.system(command)


print("[test done!]")
