import numpy as np
import sys
import time

def profile_proxy(func, *args, **kwargs) :
    t1 = time.time()
    ret = tuple(func(*args, **kwargs))
    t2 = time.time()
    print("process time : ", t2- t1 , " seconds")
    return ret

def load_volume(file_path) :
    with open(file_path, 'rb') as f :
        data = f.read()
    return data

def to_list(byte_data) : 
    return list(byte_data)

def isovalue_test(data, test, size, isovalue=0.2) :
    #for i in range(size) : 
    #    test[i] = 1 if data[i] > isovalue else 0
    test = np.array( data > isovalue )
    return test

def main() :
    volume = load_volume("./assets/dragon_vrip_FLT32_128_128_64.raw")
    volume = np.frombuffer(volume, dtype=np.float32)
    print("volume length : ", len(volume))
    volume_size = len(volume)
    volume = np.array(volume, dtype=np.float32)
    volume_test = [0 for i in range(len(volume))]
    volume_test = profile_proxy(isovalue_test, volume, volume_test, volume_size)
    print(np.sum(volume_test))

if __name__ == '__main__' :
    main()
