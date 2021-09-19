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
    return pos[2] + pos[1] * dim.x + pos[0] * dim.x * dim.y

def idx2pos(idx, dim) :
    pos = [0,0,0]
    pos[0] = idx // (dim.x * dim.y) # z
    pos[1] = ( idx - pos[0] * dim.x*dim.y ) // dim.x # y
    pos[2] = idx % dim.x # x
    return pos

def edge_test(volume, dim, isovalue=np.float32(0.2)):
    # dim = x,y,z
    gz,gy,gx = dim.z-1, dim.y-1, dim.x-1
    volume = volume.reshape((dim.z, dim.y, dim.x))
    volume = np.array(volume > isovalue, dtype=np.int32)
    volume = np.array([ volume[:gz,:gy,:gx], volume[1:dim.z, :gy, :gx], volume[:gz, 1:dim.y, :gx], volume[:gz, :gy,1:dim.x]  ], dtype=np.int32)
    out_ = np.zeros((3,dim.z, dim.y, dim.x),dtype = np.int32)
    out_[:, : gz, : gy, : gx] = np.array([volume[0] ^ volume[3]  , volume[0] ^ volume[2] , volume[0] ^ volume[1] ], dtype=np.int32)
    offset = 0
    out = np.zeros(3*dim.x*dim.y*dim.z).astype(np.int32)
    for z in range(gz) :
        for y in range(gy) :
            for x in range(gx) : 
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
                cell_types[z,y,x] = (volume[0,z,y,x] | 
                                    volume[1,z,y,x+1] | 
                                    volume[2, z,y+1,x+1] | 
                                    volume[3, z,y+1,x] | 
                                    volume[4,z+1,y,x] | 
                                    volume[5,z+1,y,x+1] | 
                                    volume[6, z+ 1, y+1, x+1] | 
                                    volume[7,z+1,y+1,x] )
                tri_counts[z,y,x] = vf.tri_counts[ cell_types[z,y,x] ]
    tri_counts = tri_counts.flatten()
    cell_types = cell_types.flatten()
    return tri_counts, cell_types

def tri_counts_prefixsum(tri_counts) :
    psum = np.cumsum(tri_counts, dtype=np.int32)
    return psum

def cast2global(pattern, dim : Dim) :
    offset = [
                0, ## 0
                3*(+1) + 1,   ## 1
                3*dim.x + 0,   ## 2
                1,   ## 3 
                3*(dim.x*dim.y) + 0,   ## 4
                3*(dim.x*dim.y + 1) + 1,   ## 5
                3*(dim.x*dim.y + dim.x) + 0,   ## 6
                3*(dim.x*dim.y) +1,   ## 7
                2,   ## 8
                3*(+1) + 2,   ## 9
                3*(dim.x + 1) + 2,   ## 10
                3*(dim.x) + 2,  ## 11
             ]
    #print('offset : ', offset)
    for i in range(len(pattern)) :
        if pattern[i] == -1 :
            break
        pattern[i] = offset[pattern[i]]
    return pattern
 
def gen_vertices(volume, et, ep, ec, dim, isovalue) :
    gx,gy,gz = dim.x,dim.y, dim.z
    vdim = Dim(gx-1,gy-1,gz-1)
    volume = volume.reshape(gz,gy,gx)
    dirs = [[0,0,1], [0,1,0], [1,0,0]]
    out = np.zeros((ep[-1], 3)  , dtype=np.float32)
    print('out size : ', out.shape)
    for i in range(ec.shape[0]) :
        eid = ec[i]
        vid = eid//3
        dv = eid % 3
        z0,y0,x0  = idx2pos(vid, Dim(dim.x-1,dim.y-1,dim.z-1))
        z1,y1,x1 = z0 + dirs[dv][0], y0 + dirs[dv][1] , x0 + dirs[dv][2]
        v0 = volume[z0,y0,x0]
        v1 = volume[z1,y1,x1]
        #print('edge_id : ', eid)
        #print("vid : ", vid)
        #print(f'p0 : {[z0, y0, x0]} value : {v0} p1 : {[z1, y1 ,x1]} value : {v1}')
        out[i] = np.array([z0, y0, x0],dtype=np.float32) + \
                   (np.array([z1, y1, x1],dtype=np.float32) - np.array([z0,y0,x0], dtype=np.float32))*((isovalue - v0)/(v1-v0) )

    return out
   
def gen_indices(volume, ct, tc, tc_psum, et_psum, dim) : 
    """
    gen index buffer
    Args :
        - ct : cell test result
        - tc : triangle count per cell
        - tc_psum : triangle count prefix sum
        - dim : dimension information
    """
    off = 0
    out = np.zeros(3*tc_psum[-1] ,dtype = np.int32)
    """
    for i in range(len(ct)) : 
        nr_tri = tc[i]
        if nr_tri == 0 :
            continue
        pattern = np.array(vf.pattern[ct[i]], dtype = np.int32)
        print(f'pattern[{ct[i]}] : ', pattern)
        pattern = cast2global(pattern, Dim(dim.x-2, dim.y-2, dim.z-2))
        print(f'converted : {pattern}')
        out[3*off : 3*(off+nr_tri)]=pattern[:3*nr_tri]
        off += nr_tri
    off = 0
    """
    for z in range(dim.z-2) :
        for y in range(dim.y-2) :
            for x in range(dim.x-2) :
                cid = pos2idx([z,y,x], Dim(dim.x-2, dim.y-2, dim.z-2))
                nr_tri =tc[cid]
                if nr_tri == 0 :
                    continue
                #print('cid : ', cid )
                #print('ctype : ', ct[cid])
                pattern = np.array(vf.pattern[ct[cid]], dtype=np.int32)
                #print('pattern : ', pattern)
                pattern = 3*pos2idx((z,y,x), Dim(dim.x-1, dim.y-1, dim.z-1)) + cast2global(pattern, Dim(dim.x-1, dim.y-1 ,dim.z-1))
                #print('converted : ', pattern)
                out[3*off : 3*(off+nr_tri)] = pattern[:3*nr_tri]
                off += nr_tri
        
    return out

def cast2unique(indices, ep) : 
    for i in range(indices.shape[0]) :
        indices[i] = ep[indices[i]]
    return indices
        

def save_object(file_name, vertices, indices) :
    with open(file_name, 'w') as f :
        for vtx in vertices :
            f.write('v {0} {1} {2}\n'.format(vtx[2], vtx[1], vtx[0]))
        for i in range(len(indices)//3) : 
            f.write('f {0} {1} {2}\n'.format(indices[3*i], indices[3*i+1], indices[3*i+2]))

def edge_test_varify(et_compact, dim) :
    """
    for i in range(et_compact.shape[0]) : 
        edge_id = et_compact[i] 
        vid = et_compact[i] // 3
        d = et_compact[i] % 3
        p0 = idx2pos(vid, Dim(dim.x-1, dim.y-1, dim.z-1))
        p1 = [ p0[0] + dirs[d][0] , p0[1] + dirs[d][1] , p0[2] + dirs[d][2]   ]
        print('edge_id : ', edge_id, 'vertex_id : ', vid)
        print('p0 : ', p0 )
        print('p1 : ', p1)
        cvt_vid = pos2idx(p0, Dim(dim.x-1, dim.y-1, dim.z-1))
        print('converted vertex id : ', cvt_vid)
    """


def main() :
    """
    volume = load_volume("./assets/dragon_vrip_FLT32_128_128_64.raw")
    dim = Dim(128, 128, 64)
    """

    volume = load_volume("./assets/Genus3_FLT32_128_64_64.raw")
    dim = Dim(128, 64, 64)
    volume = np.frombuffer(volume, dtype=np.float32)#.reshape(128,128,64)
    isovalue = 0.1
    #volume = np.zeros((4,4,4) ,dtype=np.float32)
    #volume[1,1,1] = 1.0
    print('volume max : ', np.max(volume))
    et_out = edge_test(volume, dim, isovalue=np.float32(0.2))
    print('et_out sum : ', np.sum(et_out))
    et_psum = edge_test_prefix_sum(et_out)
    print('edge_psum : ', et_psum)
    print("edge_psum[-1] : ", et_psum[-1])
    et_compact = edge_compact(et_out, et_psum)
    print('et_compact length : ', len(et_compact) )
    dirs = [[0,0,1],[0,1,0],[1,0,0]]
    
    tcnt,ctypes  = cell_test(volume, dim, isovalue = np.float32(isovalue))
    print('cell_test tri count : ', np.sum(tcnt))
    print('cell_test tri count result : ', tcnt)
    print('tri counts : ', tcnt)
    tcnt_psum = tri_counts_prefixsum(tcnt)
    print('cell test tri count psum : ', tcnt_psum[-1])
    vertices = ( gen_vertices(volume, et_out, et_psum, et_compact, dim, np.float32(isovalue)) )
    indices = gen_indices(volume, ctypes, tcnt, tcnt_psum, et_psum, dim )
    indices = cast2unique(indices, et_psum)
    print('edge psum : ', et_compact)
    save_object('sample.obj', vertices, indices)

if __name__ == '__main__' :
    main()
