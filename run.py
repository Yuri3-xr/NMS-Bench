import os
import pexpect

method = str(input())

input_file = 'data/nms_yolov8m_gpu/'
# gpu / cpu dataset is a little different, pay attention
output_file = 'data/nms_yolov8m_output' + '_' + method + '/' 


command = './build/output/main' + ' ' + input_file + ' ' + output_file  + ' ' + method
os.system(command)


print("[test done!]")
	
# transfer_command = 'sudo -S scp -rC ' + output_file + ' '+ 'sjs@10.181.8.182:/data01/sl/nms_output'  
# print(transfer_command)

# os.system(transfer_command)
# print("[transfer done!]")