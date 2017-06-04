import os
import sys
# default: testcase
folder = "testcase/testcase_v2_bug{}".format(sys.argv[3])
print folder
# default cfg=""
# 1 cfg="256 256 32 32 16 4 4 16 4 4"
# 2 cfg="512 1024 128 64 64 4 8 32 4 4"
# 3 cfg="64 32 8 16 16 4 1 16 4 4"
cfg = ""
if sys.argv[3] == "0":
	cfg=""
elif sys.argv[3] == "1":
	cfg="256 256 32 32 16 4 4 16 4 4"

elif sys.argv[3] == "2":
	cfg="512 1024 128 64 64 4 8 32 4 4"
elif sys.argv[3] == "3":
	cfg="64 32 8 16 16 4 1 16 4 4"
else:
	print "no argv"
	sys.exit(0)
	
print cfg

print "\ndelete input, output"
rm = "rm dimage.bin && rm iimage.bin && rm snapshot.rpt && rm report.rpt"
print rm
os.system(rm)

# open_testcase
# example1, example2, example3

# hidden_testcase
# array_3, easy_swap, fib_db

print "\nparameter:"
print sys.argv

t = sys.argv[1]
n = sys.argv[2]

cp1 = "cp {}/{}/{}/dimage.bin .".format(folder, t, n)
cp2 = "cp {}/{}/{}/iimage.bin .".format(folder, t, n)
os.system(cp1)
os.system(cp2)

os.system("./CMP {}".format(cfg))

cp1 = "cp snapshot.rpt {}/{}/{}/.".format(folder, t, n)
cp2 = "cp report.rpt {}/{}/{}/.".format(folder, t, n)
cp3 = "cp trace.rpt {}/{}/{}/.".format(folder, t, n)

os.system(cp1)
os.system(cp2)
os.system(cp3)

print "\ncomplete!"
