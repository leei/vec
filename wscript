import cc

def set_options(opt):
  opt.tool_options("compiler_cxx")

def configure(conf):
  conf.check_tool("compiler_cxx")
  conf.check_tool("node_addon")

def build(bld):
  ext = bld.new_task_gen("cxx", "shlib", "node_addon")
  ext.cxxflags = ["-g", "-Wall"]
  ext.source = "vec.cc bitvec.cc intvec.cc floatvec.cc"
  ext.target = "vec"

