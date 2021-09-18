import numpy as np
import sys
import time
import volume as vf

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

def edge_test(volume, dim, isovalue=np.float32(0.2)):
    # dim = x,y,z
    gz,gy,gx = dim.z-1, dim.y-1, dim.x-1
    volume = volume.reshape((dim.z, dim.y, dim.x))
    volume = np.array(volume > isovalue, dtype=np.int32)
    volume = np.array([ volume[:gz,:gy,:gx], volume[1:dim.z, :gy, :gx], volume[:gz, 1:dim.y, :gx], volume[:gz, :gy,1:dim.x]  ], dtype=np.int32)
    out_ = np.zeros((3,dim.z, dim.y, dim.x),dtype = np.int32)
    out_[:, : 63, : 127, : 127] = np.array([volume[0] ^ volume[3]  , volume[0] ^ volume[2] , volume[0] ^ volume[1] ], dtype=np.int32)
    offset = 0
    out = np.zeros(3*dim.x*dim.y*dim.z).astype(np.int32)
    for z in range(63) :
        for y in range(127) :
            for x in range(127) : 
                out[offset] = volume[0,z,y,x] ^ volume[3,z,y,x]
                out[offset+1] = volume[0,z,y,x] ^ volume[2,z,y,x]
                out[offset+2] = volume[0, z,y,x] ^ volume[1,z,y,x]
                offset += 3
    out = out.flatten()
    return out

def edge_compact(out_edge_test, out_prefix_sum) :
    nr_vertices = out_prefix_sum[-1];
    out = np.zeros(nr_vertices  ,dtype=np.int32)
    offset = 0
    print('out_edge_test length : ', out_edge_test.shape)
    print('out_prefix_sum length : ', out_prefix_sum.shape)
    for i in range(out_edge_test.shape[0]) : 
        if out_edge_test[i] : 
            out[offset] = i
            offset+=1
    return out

def edge_test_prefix_sum(in_data) : 
    return np.cumsum(in_data)

def cell_test(volume, dim, isovalue) : 
    gx = dim.x - 2
    gy = dim.y - 2
    gz = dim.z - 2
    tri_counts = np.zeros((gz, gy, gx), dtype=np.int32)
    cell_types = np.zeros((gz, gy, gx), dtype=np.int32)
    volume = np.array( volume > isovalue ,dtype=np.int32).reshape(dim.z, dim.y, dim.x)
    volume = np.array([
            volume,
            volume << 1,
            volume << 2,
            volume << 3,
            volume << 4,
            volume << 5,
            volume << 6,
            volume << 7
        ])

    for z in range(gz) : 
        for y in range(gy) : 
            for x in range(gx) :
                v0 = volume[0, z,y,x]
                pos = np.array([z,y,x],dtype=np.int32)
                cell_types[z,y,x] = (volume[0,z,y,x] | 
                                    volume[1,z,y,x+1] | 
                                    volume[2, z,y+1,x+1] | 
                                    volume[3, z,y+1,x] | 
                                    volume[4,z+1,y,x] | 
                                    volume[5,z+1,y,x+1] | 
                                    volume[6, z+ 1, y+1, x+1] | 
                                    volume[7,z+1,y+1,x] )
                tri_counts[z,y,x] = vf.tri_counts[ cell_types[z,y,x] ]

    return tri_counts, cell_types

def tri_counts_prefixsum(tri_counts) :
    psum = np.cumsum(tri_counts, dtype=np.int32)
    return psum

def gen_vertices(volume, ec, dim) :
    x,y,z = dim.x,dim.y, dim.z
    volume = volume.reshape(z,y,x)

def gen_indices(volume, ct, ct_psum, dim) : 
    pass

def main() :
    volume = load_volume("./assets/dragon_vrip_FLT32_128_128_64.raw")
    volume = np.frombuffer(volume, dtype=np.float32)#.reshape(128,128,64)
    print('volume max : ', np.max(volume))
    et_out = edge_test(volume, Dim(128,128,64), isovalue=np.float32(0.2))
    et_psum = edge_test_prefix_sum(et_out)
    et_compact = edge_compact(et_out, et_psum)
    tcnt,ctypes  = cell_test(volume, Dim(128,128,64), isovalue = np.float32(0.2))
    tcnt_psum = tri_counts_prefixsum(tcnt)
    
    print('et_out sum : ', np.sum(et_out))
    print("edge_psum[-1] : ", et_psum[-1])
    print('et_compact sum : ', np.sum(et_compact) )
    for i in range(0, 10) : 
        print(et_compact[i])

    print('tri_counts_sum : ', np.sum(tcnt))
    print('tri_counts_psum[-1] : ', tcnt_psum[-1])

if __name__ == '__main__' :
    main()
