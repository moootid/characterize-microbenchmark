import csv
import os
import matplotlib.pyplot as plt
import pandas as pd

runtime_type= ["scalar", "vectorized"]
file_path = "_runtimes.csv"
data_size_title = ["defualt", "test","dev" ,"small", "medium", "large", "native"]
data_size = [4, 4,23 ,4*1000, 16*1000, 64*1000, 10 * 1000 * 1000]
scalar_runtime = []
vec_runtime = []
vec_semulated_runtime = []
scalar_semulated_runtime = []
semilated_data_size = [4, 4,23 ,4*1000, 16*1000, 64*1000]
operation_speed = 0.0

for i in range(len(runtime_type)):
    for j in range(len(data_size_title)):
        file_path = runtime_type[i] + "_" + data_size_title[j] + "_runtimes.csv"
        if not os.path.exists(file_path):
            print("Error: File does not exist")
        else:
            with open(file_path, "r") as file:
                csv_reader = csv.reader(file)
                data_list = list(csv_reader)
                if runtime_type[i] == "scalar":
                    scalar_runtime.append(int(data_list[3][1]))
                    vec_semulated_runtime.append(int(data_list[3][1]))
                elif runtime_type[i] == "vectorized":
                    vec_runtime.append(int(data_list[3][1]))
                    scalar_semulated_runtime.append(int(data_list[3][1]))

for i in range(3,len(vec_runtime)):
    operation_speed += vec_runtime[i] / data_size[i]
    if i == len(vec_runtime)-1:
        operation_speed = operation_speed / 4
vec_semulated_runtime.pop()
scalar_semulated_runtime.pop()

for i in range(64*1000,10 * 1000 * 1000, 64*1000):
    vec_semulated_runtime.append(int(i * operation_speed) )
    semilated_data_size.append(i)
vec_operation_speed = operation_speed

for i in range(3,len(scalar_runtime)):
    operation_speed += scalar_runtime[i] / data_size[i]
    if i == len(scalar_runtime)-1:
        operation_speed = operation_speed / 4
        
scalar_operation_speed = operation_speed

for i in range(64*1000,10 * 1000 * 1000, 64*1000):
    scalar_semulated_runtime.append(int(i * operation_speed) )
    
df = pd.DataFrame(columns = ['scalar', 'vectorized', 'data_size'])
df["scalar"] = scalar_semulated_runtime
df["vectorized"] = vec_semulated_runtime
df["data_size"] = semilated_data_size
df.to_csv('semulated_runtimes.csv')
plt.plot(data_size_title, scalar_runtime, label='Scalar')
plt.plot(data_size_title, vec_runtime, label='Vectorized')


plt.xlabel('Data Size')
plt.ylabel('Runtime [ms/10]')
plt.title('Runtime per Data Size ')
plt.legend()
plt.show()

for i in range(len(semilated_data_size)):
    semilated_data_size[i] = semilated_data_size[i] / 1000
plt.plot(semilated_data_size, scalar_semulated_runtime, label='Scalar Simulated')
plt.plot(semilated_data_size, vec_semulated_runtime, label='Vectorized Simulated')


plt.xlabel('Data Size [n/1000]')
plt.ylabel('Runtime')
plt.title('Runtime per Data Size (recreated)')
plt.legend()
plt.show()

print("-------------------")
print ("Data:")
print(" Scalar average runtime (For one row):", round(scalar_operation_speed,2), "ns")
print(" Vectorized average runtime (For one row):", round(vec_operation_speed,2),"ns")
print(" Speedup in vectorized:",  round((vec_operation_speed - scalar_operation_speed)/scalar_operation_speed*100*-1,2), "%")