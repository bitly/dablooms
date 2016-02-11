package dablooms

/*
#cgo LDFLAGS: -ldablooms

#include <stdlib.h>
#include <dablooms.h>
*/
import "C"

import (
	"unsafe"
)

func Version() string {
	return "0.9.0"
}

type ScalingBloom struct {
	cfilter *C.scaling_bloom_t
}

func NewScalingBloom(capacity C.uint, errorRate C.double, filename string) *ScalingBloom {
	cFilename := C.CString(filename)
	defer C.free(unsafe.Pointer(cFilename))
	sb := &ScalingBloom{
		cfilter: C.new_scaling_bloom(capacity, errorRate, cFilename),
	}
	return sb
}

func NewScalingBloomFromFile(capacity C.uint, errorRate C.double, filename string) *ScalingBloom {
	cFilename := C.CString(filename)
	defer C.free(unsafe.Pointer(cFilename))
	sb := &ScalingBloom{
		cfilter: C.new_scaling_bloom_from_file(capacity, errorRate, cFilename),
	}
	return sb
}

// apparently this is an unsupported feature of cgo
// we should probably use runtime.SetFinalizer
// see: https://groups.google.com/forum/?fromgroups#!topic/golang-dev/5cD0EmU2voI
func (sb *ScalingBloom) Destroy() {
	C.free_scaling_bloom(sb.cfilter)
}

func (sb *ScalingBloom) Check(key []byte) bool {
	cKey := (*C.char)(unsafe.Pointer(&key[0]))
	return C.scaling_bloom_check(sb.cfilter, cKey, C.size_t(len(key))) == 1
}

func (sb *ScalingBloom) Add(key []byte, id C.uint64_t) bool {
	cKey := (*C.char)(unsafe.Pointer(&key[0]))
	return C.scaling_bloom_add(sb.cfilter, cKey, C.size_t(len(key)), id) == 1
}

func (sb *ScalingBloom) Remove(key []byte, id C.uint64_t) bool {
	cKey := (*C.char)(unsafe.Pointer(&key[0]))
	return C.scaling_bloom_remove(sb.cfilter, cKey, C.size_t(len(key)), id) == 1
}

func (sb *ScalingBloom) Flush() bool {
	return C.scaling_bloom_flush(sb.cfilter) == 1
}

func (sb *ScalingBloom) MemSeqNum() C.uint64_t {
	return C.scaling_bloom_mem_seqnum(sb.cfilter)
}

func (sb *ScalingBloom) DiskSeqNum() C.uint64_t {
	return C.scaling_bloom_disk_seqnum(sb.cfilter)
}
