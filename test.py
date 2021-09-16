import numpy as np
import sys
import time

class Dim :
    def __init__(self,x,y,z) :
        self.x = x
        self.y = y
        self.z = z

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
    test = np.array( data > isovalue , astype=np.int32)
    return test

def pos2idx(pos, dim) : 
    return pos.x + pos.y * dim.x + pos.z * dim.x * dim.y

def edge_test(data, dim, isovalue=np.float32(0.2)):
    # dim = x,y,z
    print('volume shape : ', data.shape)
    out = np.zeros(dim.x * dim.y * dim.z * 3, dtype=np.int32)
    vdim = Dim(dim.x+1, dim.y+1, dim.z+1)
    for z in range(dim.z) : 
        for y in range(dim.y) :
            for x in range(dim.x) : 
                p0_id = pos2idx(Dim(x,y,z), vdim)
                o = data[p0_id] > isovalue
                o_id = 3*pos2idx(Dim(x,y,z), dim)
                out[o_id] = o ^ (data[ p0_id + pos2idx(Dim(1,0,0), vdim) ] > isovalue)
                out[o_id+1] = o ^ (data[ p0_id + pos2idx(Dim(0,1,0), vdim) ] > isovalue)
                out[o_id+2] = o ^ (data[ p0_id + pos2idx(Dim(0,0,1), vdim) ] > isovalue)
                #print('o_id : ', o_id)
    print(out)
    print("edge_test sum : ", np.sum(out) )
    return out

def edge_compact(out_edge_test, out_prefix_sum) :
    nr_vertices = out_prefix_sum[-1];
    out = np.zeros(nr_vertices  ,dtype=np.int32)
    offset = 0
    print('out_edge_test length : ', out_edge_test.shape)
    print('out_prefix_sum length : ', out_prefix_sum.shape)
    for i in range(out_edge_test.shape[0]) : 
        if out_edge_test[i] : 
            out[offset] = i-1
            offset+=1
    return out

def edge_test_prefix_sum(in_data) : 
    return np.cumsum(in_data)

def main() :
    volume = load_volume("./assets/dragon_vrip_FLT32_128_128_64.raw")
    volume = np.frombuffer(volume, dtype=np.float32)#.reshape(128,128,64)
    print('volume max : ', np.max(volume))
    #print("volume length : ", len(volume)*4)
    #volume_size = len(volume)
    #volume_test = [0 for i in range(len(volume))]
    #volume_test = profile_proxy(isovalue_test, volume, volume_test, volume_size)
    #print(np.sum(volume_test))
    #volume = np.zeros((3,3,3), dtype = np.float32)
    #volume[1,1,1] = 1
    et_out = edge_test(volume, Dim(127,127,63), isovalue=np.float32(0.2))
    et_psum = edge_test_prefix_sum(et_out)
    et_compact = edge_compact(et_out, et_psum)
    
    print('et_out sum : ', np.sum(et_out))
    print("edge_psum[-1] : ", et_psum[-1])
    print('et_compact sum : ', np.sum(et_compact) )
    print('et_compact : ', et_compact)

if __name__ == '__main__' :
    main()
