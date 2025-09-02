AudioSignalResClass {
 Inputs {
  IOPItemInputClass {
   id 1
   name "Offset"
   tl -16 -3
   children {
    2
   }
   valueMax 300000
  }
  IOPItemInputClass {
   id 12
   name "SampleIndex"
   tl -20.5 103
   children {
    11
   }
   valueMax 300000
  }
 }
 Outputs {
  IOPItemOutputClass {
   id 2
   name "Offset"
   tl 243 1
   input 1
  }
  IOPItemOutputClass {
   id 11
   name "SampleIndex"
   tl 230.5 107
   input 12
  }
 }
 compiled IOPCompiledClass {
  visited {
   133 134 5 6
  }
  ins {
   IOPCompiledIn {
    data {
     1 2
    }
   }
   IOPCompiledIn {
    data {
     1 65538
    }
   }
  }
  outs {
   IOPCompiledOut {
    data {
     0
    }
   }
   IOPCompiledOut {
    data {
     0
    }
   }
  }
  processed 4
  version 2
 }
}