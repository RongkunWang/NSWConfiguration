# This script runs a 191 development version of the baseline analysis code (nn191fast)

import os
import time
import argparse
import ROOT
ROOT.gROOT.SetBatch()

def options():
    parser = argparse.ArgumentParser(usage=__doc__, formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("-t", help="Timestamp used for directory and file names")
    parser.add_argument("-i", help="Input txt file of baseline measurements")
    parser.add_argument("-r", help="Input root file of baseline measurements. This is currently not working!")
    parser.add_argument("-o", help="Output directory", default="")
    parser.add_argument("-y", help="y-axis range maximum for profile plots", default="5")
    parser.add_argument("-n", help="Comma-separated list of MMFE8s")
    parser.add_argument("-g", help="Grouping parameter for output plots (default is no MMFE8 plots")
    parser.add_argument("-u", help="Units of measurements (mV or ENC)", default="ENC")
    return parser.parse_args()

def main():

    ops = options()
    if not ops.t:
        fatal("Please give an timestamp with -t")
    if not ops.i:
        fatal("Please give an input txt file with -i")
#    if not ops.i and not ops.r:
#        fatal("Please give an input txt file with -i or an input root file with -r")
    if ops.g:
        if not ops.g in ["l", "L", "layer", "Layer", "p", "P", "pcb", "PCB"]:
            fatal("Please give a grouping parameter -g like `pcb` or `layer`")
    if not ops.u in ["mV", "ENC"]:
        fatal("Please give a unit -u as mV or ENC")
    if ops.u == "ENC" and ops.y == "5":
        ops.y = "4000"

    print(ops.t)

    rootlogon()

#    parse febs from input
    febs = []
    if not ops.n:
        lines = open(ops.i).readlines()
        febs = sorted( list( set( [ line.split()[1] for line in lines ] ) ) )
    else:
        febs = ops.n.split(",")
    for feb in febs:
        print("Found %s" % (feb))
    

    if ops.i:
        out = ROOT.TFile(ops.o+"/ntuple/baselines_ntuple.root", "recreate")
        # txt -> root
        print "Loading data..."
        tr = ROOT.TTree("DATA", "")
        tr.ReadFile(ops.i, "data/C:MMFE8:vmm_id/D:channel_id:tpdac:thdac:trim:ADCsample", " ")
        out.cd()
        tr.Write()
        print "Data loaded."
    elif ops.r:
        # copy the input file to output and update: THIS IS CURRENTLY NOT WORKING
        cmd = "cp -f "+ops.r+" "+ops.o
        os.system(cmd)  
        out = ROOT.TFile(ops.o, "")
        outdir = out.GetDirectory("elx_report")
        outdir.Delete("*;*")
        # copy the tree into the new file
        print "Copying the ntuple..."
        inputrootfile = ROOT.TFile(ops.r, "read")
        tr = inputrootfile.Get("DATA")
        out.cd()
#        tr.AutoSave()
        inputrootfile.Close()
        print "Done."

# make MMFE8 plots only if requested (by requesting a grouping)
    if ops.g:
        mmfe8plots(febs,ops,out,tr)

    out.Close() 

    print(out)
    print("The ntuple root file is CLOSED")

##    os.system("root -l -b nn191dev/run.cpp  -q")
    os.system("root -l -b 'nn191fast/run.cpp+(\""+ops.t+"\")'  -q")

def mmfe8plots(febs,ops,out,tr):
    # debugging
    rms_of_interest = 50000.0 # mV
    
    # plot each feb
    for feb in febs:
        if not feb:
            continue
            
        name = "baseline_%s" % (feb)

        # output TDirectory
        layer, pcb, quad = layer_pcb_quad(feb)
        if   ops.g in ["l", "L", "layer", "Layer"]: outdirname = "Layer_%s_%s" % (layer, quad)
        elif ops.g in ["p", "P", "pcb", "PCB"]:     outdirname = "PCB_%s" % (pcb)
        else:
            fatal("Please provide a --group which is `layer` or `pcb`")
        outdir = out.GetDirectory(outdirname)
        if not outdir:
            outdir = out.mkdir(outdirname)
        febdir = outdir.mkdir(feb)

        # channel baseline
        hist = ROOT.TH2D(name, ";VMM*64 + Channel;Baseline [mV]; Samples", 512, -0.5, 511.5, 290, 10, 300)
        tr.Draw("ADCsample/4095 * 1.5 * 1000 : channel_id + 64*vmm_id >> %s" % (name), "strstr(MMFE8,\"%s\")" % (feb), "colz")
        style(hist)
        canv = ROOT.TCanvas(name+"_canv", name+"_canv", 800, 800)
        canv.Draw()
        hist.Draw("colzsame")
        outdir.cd()
        febdir.cd()
        canv.Write()
        canv.Close()

        
        # create VMM boundaries
        lines = []
        for vmm in range(1, 8):
            x = vmm*64
            lines.append( ROOT.TLine(x, 0, x, float(ops.y)) )
            lines[-1].SetLineColor(17)
            lines[-1].SetLineStyle(7)

        # channel RMS (profile)
        tmpx = hist.ProfileX("_tmpX", 1, hist.GetNbinsY(), "s")
        prof = ROOT.TH1D(name+"_prof", ";VMM*64 + Channel;Baseline RMS [%s]" % (ops.u), 512, -0.5, 511.5)
        for bin in range(0, tmpx.GetNbinsX()+1):
            multiplier = 1.0 if ops.u=="mV" else 6241.50975/9.0
            prof.SetBinContent(bin, tmpx.GetBinError(bin) * multiplier)
            prof.SetBinError  (bin, 0)
            if prof.GetBinContent(bin) > rms_of_interest:
                # bin 1 is channel 0
                print_me = "%s Bin %03i has RMS %4.1f => VMM/CH = %i/%02i"
                print print_me % (feb, (bin-1), prof.GetBinContent(bin), (bin-1)/64, (bin-1)%64)
                # print "%s Bin %03i has RMS %4.1f => VMM/CH = %i/%02i" 
                # % (feb, (bin-1), prof.GetBinContent(bin), (bin-1)/64, (bin-1)%64)
            #if prof.GetBinContent(bin) > float(ops.y):
            #    prof.SetBinContent(bin, float(ops.y)-0.1)
        style(prof)
        prof.SetMinimum(0)
        prof.SetMaximum(float(ops.y))
        prof.SetLineWidth(0)
        prof.SetMarkerStyle(ROOT.kFullCircle)
        prof.SetMarkerSize(0.7)
        prof.SetMarkerColor(ROOT.kBlack)


        median_board = sorted([prof.GetBinContent(bin) for bin in xrange(1, 512+1)])
        median_value = sorted([prof.GetBinContent(bin) for bin in xrange(1, 512+1)]) [512/2]
        mean_board_value = 0
        for bin in xrange(1,512+1):
            mean_board_value = prof.GetBinContent(bin) + mean_board_value
        mean_value = mean_board_value/512
        if median_board[-1] > mean_value + 500:
            max_fit_val = mean_value + 500
        else: max_fit_val = median_board[-1]
        if median_board[0] < median_value - 500:
            min_fit_val = mean_value - 500
        else: min_fit_val = mean_value - 500
        hist_gauss = ROOT.TH1D(name, ";ADC; Events", 100, min_fit_val, max_fit_val)
        for bin in range(0, 512):
            hist_gauss.Fill(median_board[bin])
        gaussFit = ROOT.TF1("gaussfit","gaus",median_board[0],median_board[-1])
        hist_gauss.Fit(gaussFit,"EQ")
        hist_gauss.Draw("")
#        canvas.Print("%s_trial_gaus.pdf"%name)                                                                                                                                          
        down_limit_sigma = gaussFit.GetParameter(1) - 6*gaussFit.GetParameter(2)
        up_limit_sigma = gaussFit.GetParameter(1) + 6*gaussFit.GetParameter(2)


# Henri: removing the Gaussian fit and window around the median:
#        median_board = sorted([prof.GetBinContent(bin) for bin in xrange(1, 512+1)])
#        median_value = median_board[512/2]
#        mean_board_value = 0
#        for bin in xrange(1,512+1):
#            mean_board_value = prof.GetBinContent(bin) + mean_board_value
#        mean_value = mean_board_value/512
#        if median_board[-1] > mean_value + 500:
#            max_fit_val = mean_value + 500
#        else: max_fit_val = median_board[-1]
#        if median_board[0] < median_value - 500:
#            min_fit_val = mean_value - 500
#        else: min_fit_val = mean_value - 500
#        hist_gauss = ROOT.TH1D(name, ";ADC; Events", 100, min_fit_val, max_fit_val)
#        for bin in range(0, 512):
#            hist_gauss.Fill(median_board[bin])
#        gaussFit = ROOT.TF1("gaussfit","gaus",median_board[0],median_board[-1])
#        hist_gauss.Fit(gaussFit,"EQ")
#        hist_gauss.Draw("")
##        canvas.Print("%s_trial_gaus.pdf"%name)                                                                                                                                          
#        down_limit_sigma = gaussFit.GetParameter(1) - 6*gaussFit.GetParameter(2)
#        up_limit_sigma = gaussFit.GetParameter(1) + 6*gaussFit.GetParameter(2)


        # VMM mean/median
        latexs = []
        for vmm in range(0, 8):
            x = vmm*64
            mean   = prof.Integral(x+1, x+64) / 64.0
            median = sorted([prof.GetBinContent(bin) for bin in xrange(x+1, x+64+1)]) [64/2]
            val    = median # change me at your desire
            string = ("%.2f" % (val)) if ops.u=="mV" else str(int(val))
            latexs.append( ROOT.TLatex(x+64/2, float(ops.y)*1.02, string) )
            latexs[-1].SetTextSize(0.025)
            latexs[-1].SetTextFont(42)
            latexs[-1].SetTextAlign(21)
            lines.append( ROOT.TLine(x, val, x+64, val) )
            lines[-1].SetLineColor(ROOT.kBlack)
            lines[-1].SetLineStyle(1)
            lines[-1].SetLineWidth(1)
            down_line_sigma = ROOT.TLine(0,down_limit_sigma, 512, down_limit_sigma)
            up_line_sigma = ROOT.TLine(0,up_limit_sigma, 512, up_limit_sigma)
            down_line_sigma.SetLineColor(ROOT.kRed)
            up_line_sigma.SetLineColor(ROOT.kRed)


        # clone for overflow
        p_up = prof.Clone(prof.GetName()+"_clone_up")
        for bin in range(0, p_up.GetNbinsX()+1):
            if p_up.GetBinContent(bin) > float(ops.y):
                p_up.SetBinContent(bin, float(ops.y)-0.01)
            else:
                p_up.SetBinContent(bin, -1)
        style(p_up)
        p_up.SetMinimum(0)
        p_up.SetMaximum(float(ops.y))
        p_up.SetMarkerStyle(ROOT.kFullTriangleUp)
        p_up.SetMarkerColor(ROOT.kRed)
        p_up.SetMarkerSize(2.0)

        # clone for underflow
        p_dn = prof.Clone(prof.GetName()+"_clone_dn")
        for bin in range(0, p_dn.GetNbinsX()+1):
            if p_dn.GetBinContent(bin) < 0.1:
                p_dn.SetBinContent(bin, 0.01)
            else:
                p_dn.SetBinContent(bin, -1)
        style(p_dn)
        p_dn.SetMinimum(0)
        p_dn.SetMaximum(float(ops.y))
        p_dn.SetMarkerStyle(ROOT.kFullCircle)
        p_dn.SetMarkerColor(ROOT.kRed)

        # sigma implementation                                                                                                                                                           
        count_down_sigma = 0
        count_up_sigma = 0
        p_sigma = prof.Clone(prof.GetName()+"_sigma")
        for bin in range(1, p_sigma.GetNbinsX()+1):
            if p_sigma.GetBinContent(bin) > up_limit_sigma:
                count_up_sigma = count_up_sigma + 1
                if (name[16] == "3" and name[18] == "5" and name[20] == "H" and name[22] == "L") or (name[16] == "3" and name[18] == "6" and name[20] == "H" and name[22] == "R") or (name[16] == "3" and name[18] == "5" and name[20] == "I" and name[22] == "R") or (name[16] == "3" and name[18] == "6" and name[20] == "I" and name[22] == "L") or (name[16] == "3" and name[18] == "8" and name[20] == "I" and name[22] == "R") or (name[16] == "4" and name[18] == "5" and name[20] == "H" and name[22] == "R") or (name[16] == "4" and name[18] == "6" and name[20] == "H" and name[22] == "L") or (name[16] == "4" and name[18] == "8" and name[20] == "H" and name[22] == "R") or (name[16] == "4" and name[18] == "5" and name[20] == "I" and name[22] == "L") or (name[16] == "4" and name[18] == "6" and name[20] == "I" and name[22] == "R") or (name[16] == "4" and name[18] == "8" and name[20] == "I" and name[22] == "L") or (name[16] == "4" and name[18] == "6" and name[20] == "H" and name[22] == "L") or (name[16] == "4" and name[18] == "8" and name[20] == "I" and name[22] == "L"):
                    down_limit_sigma = 1000
                    down_line_sigma = ROOT.TLine(0,down_limit_sigma, 512, down_limit_sigma)
                    down_line_sigma.SetLineColor(ROOT.kRed)
            if p_sigma.GetBinContent(bin) < down_limit_sigma:
                p_sigma.SetBinContent(bin,p_sigma.GetBinContent(bin))
                count_down_sigma = count_down_sigma + 1
            else:
                p_sigma.SetBinContent(bin,-1.5)
            if p_sigma.GetBinContent(bin) == 0:
                p_sigma.SetBinContent(bin,1.5)


        #  missing channels                                                                                                                                                             

        missing_channels_distr = prof.Clone(prof.GetName()+"_missing")
        number_miss_channels = missing_channels(name)
        for bin in range(1, p_sigma.GetNbinsX()+1):

                if 513 - bin > number_miss_channels:
                    missing_channels_distr.SetBinContent(bin,-1.5)
                else:
                    missing_channels_distr.SetBinContent(bin,missing_channels_distr.GetBinContent(bin))
                    if p_sigma.GetBinContent(bin) < down_limit_sigma:
                        count_down_sigma = count_down_sigma - 1


        style(p_sigma)
        p_sigma.SetMinimum(0)
        p_sigma.SetMaximum(float(ops.y))
        p_sigma.SetMarkerStyle(ROOT.kFullCrossX)
        p_sigma.SetMarkerColor(ROOT.kBlue)
        p_sigma.SetMarkerSize(1.2)
        word_dead = ("Dead = %i \n"%count_down_sigma)
        word_noisy = ("Noisy = %i "%count_up_sigma)
        word_total = ("Total = %i "%(count_up_sigma+count_down_sigma))
        dead_counter_latex = ROOT.TLatex(75,float(ops.y)*0.95,word_dead)
        dead_counter_latex.SetTextSize(0.035)
        dead_counter_latex.SetTextColor(2)

        noisy_counter_latex = ROOT.TLatex(75,float(ops.y)*0.9,word_noisy)
        noisy_counter_latex.SetTextSize(0.035)
        noisy_counter_latex.SetTextColor(2)

        total_counter_latex = ROOT.TLatex(75,float(ops.y)*0.85,word_total)
        total_counter_latex.SetTextSize(0.035)
        total_counter_latex.SetTextColor(2)


        style(missing_channels_distr)
        missing_channels_distr.SetMinimum(0)
        missing_channels_distr.SetMaximum(float(ops.y))
        missing_channels_distr.SetMarkerStyle(ROOT.kFullCrossX)
        missing_channels_distr.SetMarkerColor(ROOT.kOrange)
        missing_channels_distr.SetMarkerSize(1.2)


        # draw                                                                                                                                                                           
        canv = ROOT.TCanvas(name+"_prfx", name+"_prfx", 800, 800)
        canv.Draw()
        p_sigma.Draw("psame")
        p_up.Draw("psame")
        missing_channels_distr.Draw("psame")
        p_dn.Draw("psame")
        down_line_sigma.Draw()
        up_line_sigma.Draw()
        if count_down_sigma > 10:
            canv.SetFrameFillColor(10)
            canv.SetFillColor(2)
            dead_counter_latex.SetTextColor(2)
        for line in lines:
            line.Draw()
        for latex in latexs:
            latex.Draw()
        dead_counter_latex.Draw()
        noisy_counter_latex.Draw()
        total_counter_latex.Draw()
        prof.Draw("psame")
        outdir.cd()
        febdir.cd()
        canv.Write()
        canv.Close()

        # VMM RMS (profile)
        # ROOT.gStyle.SetPadRightMargin(0.06)
        # vmm = hist.RebinX(64)
        # prof = vmm.ProfileX("_profileX_VMM", 1, vmm.GetNbinsY(), "s")
        # prof.GetYaxis().SetTitle("VMM Baseline RMS [mV]")
        # prfx = ROOT.TH1D(name+"_prfx_vmm", ";VMM;VMM Baseline RMS [mV]", 8, -0.5, 7.5)
        # for bin in range(0, prof.GetNbinsX()+1):
        #     prfx.SetBinContent(bin, prof.GetBinError(bin))
        #     prfx.SetBinError  (bin, 0)
        # style(prfx)
        # prfx.SetMinimum(0)
        # canv = ROOT.TCanvas(name+"_vmm", name+"_vmm", 800, 800)
        # canv.Draw()
        # prfx.Draw("same")
        # canv.SaveAs(canv.GetName() + ".pdf")
        # outdir.cd()
        # canv.Write()
        # ROOT.gStyle.SetPadRightMargin(0.19)

def rootlogon():
    ROOT.gStyle.SetOptStat(0)
    ROOT.gStyle.SetPadTopMargin(0.06)
    ROOT.gStyle.SetPadRightMargin(0.19)
    ROOT.gStyle.SetPadBottomMargin(0.12)
    ROOT.gStyle.SetPadLeftMargin(0.16)
    ROOT.gStyle.SetPadTickX(1)
    ROOT.gStyle.SetPadTickY(1)
    ROOT.gStyle.SetPaintTextFormat(".2f")
    ROOT.gStyle.SetTextFont(42)
    ROOT.gStyle.SetFillColor(10)

def style(hist):
    size = 0.045
    hist.SetLineWidth(2)
    hist.GetXaxis().SetTitleSize(size - 0.025)
    hist.GetXaxis().SetLabelSize(size-0.025)
    hist.GetYaxis().SetTitleSize(size - 0.025)
    hist.GetYaxis().SetLabelSize(size - 0.025)
    hist.GetZaxis().SetTitleSize(size)
    hist.GetZaxis().SetLabelSize(size)
    hist.GetXaxis().SetTitleOffset(1.1)
    hist.GetYaxis().SetTitleOffset(1.8)
    hist.GetZaxis().SetTitleOffset(1.6)

    
def layer_pcb_quad(feb):
    # LxPy_zz e.g. L1P6_IPR                                                                                                                                                              
    import re
    result = re.findall("L\dP\d_\w\w", feb)
    if not result:
        fatal("Couldnt find the LxPy pattern in %s" % (feb))
    if len(result) > 1:
        fatal("Tried to find the LxPy pattern, but got confused: %s" % (feb))
    first = result[0]
    _, layer, _, pcb, _, q0, q1 = first
    return (layer, pcb, q0+q1)


def missing_channels(name):
    miss_sm1_p1_e = 42
    miss_sm1_p5_e = 41
    miss_sm2_p6_e = 29
    miss_sm2_p8_e = 31

    miss_sm1_p1_s = 35
    miss_sm1_p5_s = 11
    miss_sm2_p6_s = 1
    miss_sm2_p8_s = 1

    miss_lm1_p1_e = 72
    miss_lm1_p5_e = 72
    miss_lm2_p6_e = 47
    miss_lm2_p8_e = 48

    miss_lm1_p1_s = 86
    miss_lm1_p5_s = 36
    miss_lm2_p6_s = 42
    miss_lm2_p8_s = 43

    if name[16] == "1" or name[16] == "2":
        if name[18] == "1":
            number_miss_channels = miss_sm1_p1_e
        elif name[18] == "5":
            number_miss_channels = miss_sm1_p5_e
        elif name[18] == "6":
            number_miss_channels = miss_sm2_p6_e
        elif name[18] == "8":
            number_miss_channels = miss_sm2_p8_e
        else:
            number_miss_channels = 0
            
    
    else:
        if name[18] == "1":
            number_miss_channels = miss_sm1_p1_s
        elif name[18] == "5":
            number_miss_channels = miss_sm1_p5_s
        elif name[18] == "6":
            number_miss_channels = miss_sm2_p6_s
        elif name[18] == "8":
            number_miss_channels = miss_sm2_p8_s
        else:
            number_miss_channels = 0

    if((name[16] == "1" or name[16] == "3") and (name[22] == "R") and (name[20] == "I") and (name[18] == "1")):
        number_miss_channels = 0
    elif((name[16] == "1" or name[16] == "3") and (name[22] == "L") and (name[20] == "I") and (name[18] == "5")):
        number_miss_channels = 0
    elif((name[16] == "1" or name[16] == "3") and (name[22] == "R") and (name[20] == "I") and (name[18] == "6")):
        number_miss_channels = 0
    elif((name[16] == "1" or name[16] == "3") and (name[22] == "L") and (name[20] == "I") and (name[18] == "8")):
        number_miss_channels = 0
    elif((name[16] == "2" or name[16] == "4") and (name[22] == "L") and (name[20] == "I") and (name[18] == "1")):
        number_miss_channels = 0
    elif((name[16] == "2" or name[16] == "4") and (name[22] == "R") and (name[20] == "I") and (name[18] == "5")):
        number_miss_channels = 0
    elif((name[16] == "2" or name[16] == "4") and (name[22] == "L") and (name[20] == "I") and (name[18] == "6")):
        number_miss_channels = 0
    elif((name[16] == "2" or name[16] == "4") and (name[22] == "R") and (name[20] == "I") and (name[18] == "8")):
        number_miss_channels = 0
    elif((name[16] == "1" or name[16] == "3") and (name[22] == "L") and (name[20] == "H") and (name[18] == "1")):
        number_miss_channels = 0
    elif((name[16] == "1" or name[16] == "3") and (name[22] == "R") and (name[20] == "H") and (name[18] == "5")):
        number_miss_channels = 0
    elif((name[16] == "1" or name[16] == "3") and (name[22] == "L") and (name[20] == "H") and (name[18] == "6")):
        number_miss_channels = 0
    elif((name[16] == "1" or name[16] == "3") and (name[22] == "R") and (name[20] == "H") and (name[18] == "8")):
        number_miss_channels = 0
    elif((name[16] == "2" or name[16] == "4") and (name[22] == "R") and (name[20] == "H") and (name[18] == "1")):
        number_miss_channels = 0
    elif((name[16] == "2" or name[16] == "4") and (name[22] == "L") and (name[20] == "H") and (name[18] == "5")):
        number_miss_channels = 0
    elif((name[16] == "2" or name[16] == "4") and (name[22] == "R") and (name[20] == "H") and (name[18] == "6")):
        number_miss_channels = 0
    elif((name[16] == "2" or name[16] == "4") and (name[22] == "L") and (name[20] == "H") and (name[18] == "8")):
        number_miss_channels = 0
    return (number_miss_channels)



def fatal(msg):
    import sys
    sys.exit("Fatal error: %s" % (msg))

if __name__ == "__main__":
    main()
