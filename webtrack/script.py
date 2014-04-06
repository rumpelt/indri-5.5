import subprocess
import sys
def runBasicClue9(runName, serverfile):
  
    prog = ['./IndriRunQuery','--param', serverfile,  '--param', 'query/query.clue9.basic', '--basic-run' , 'true']
    prog.append('--qfile=topics/clue12.topic.full.xml')
    rulestr = '--r-param=<parameters><rule>method:dirichlet,mu:'
    outFile=runName
    mu = 3000
    
    while mu < 8000:
        rule = rulestr+str(mu)+'</rule></parameters>'
        args = prog[:] #makes a copy
        args.append(rule)
        args.append('--dfile')
        outfile = runName +'-mu'+str(mu)
        args.append(outfile)
        #print args
        pchild = subprocess.Popen(args, stderr=subprocess.STDOUT)
        pchild.wait() 
        mu = mu + 500

def runclue12(runName, serverfile):
    """
    runName : the outputfile which will be created
    serverfile : the parameter file containing the server to use for query (disk based or indri daemons)
    """
    prog = ['./IndriRunQuery','--param', serverfile, '--param', 'stop.param', '--param', 'queries.web2013.basic']
    dumpfile = 'run/' + runName
    psgSz =  150
    while psgSz <= 1000:
        mu = 500
        while mu <= 5000:
            outfile = dumpfile + '-'+str(mu)+'-'+str(psgSz)
            prog_ex = prog[:] # creates a new copy
            prog_ex.extend(['--mu', str(mu), '--psg', str(psgSz), '--dfile', outfile])
            print prog_ex
            pchild = subprocess.Popen(prog_ex, stderr=subprocess.STDOUT)
            pchild.wait()
            mu = mu + 500
        psgSz = psgSz + 100

if __name__ == '__main__':
   # sys.exit(runclue12(sys.argv[1], sys.argv[2]))
    sys.exit(runBasicClue9(sys.argv[1], sys.argv[2]))
