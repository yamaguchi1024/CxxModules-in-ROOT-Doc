void memory()
{
  ProcInfo_t info;
  gSystem->GetProcInfo(&info);
  printf(" cpu  time = %f seconds\n",info.fCpuUser);
  printf(" sys  time = %f seconds\n",info.fCpuSys);
  printf(" res  memory = %g Mbytes\n",info.fMemResident/1024.);
  printf(" vir  memory = %g Mbytes\n",info.fMemVirtual/1024.);
}
