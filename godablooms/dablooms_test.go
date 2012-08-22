package dablooms

import (
	"io/ioutil"
	"log"
	"os"
	"testing"
)

func TestPutMessage(t *testing.T) {
	log.SetOutput(ioutil.Discard)
	defer log.SetOutput(os.Stdout)

	sb := NewScalingBloom(1000, 0.5, "testbloom.bin")
	if sb == nil {
		t.Fatalf("NewScalingBloom failed")
	}
	
	key := []byte("test")

	if sb.Check(key) != false {
		t.Fatalf("Check failed")
	}

	if sb.Add(key, 1) != true {
		t.Fatalf("Add failed")
	}

	if sb.Check(key) != true {
		t.Fatalf("'%s' not found", key)
	}

	if sb.Remove(key, 1) != true {
		t.Fatalf("Remove failed")
	}
	
	if sb.Check(key) != false {
		t.Fatalf("'%s' was found (after remove)", key)
	}
}
