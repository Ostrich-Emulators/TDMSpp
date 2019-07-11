from nptdms import TdmsFile

tdms_file = TdmsFile("../FormatConverter/test.medi")
channel = tdms_file.object('Intellivue', 'CmpndECG(I)')
data = channel.data

for val in data:
  print("{0:5.4f}".format(val))
