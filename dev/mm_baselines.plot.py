import argparse
import ROOT
ROOT.gROOT.SetBatch()

def options():
    parser = argparse.ArgumentParser(usage=__doc__, formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("-i", help="Input txt file of baseline measurements")
    parser.add_argument("-o", help="Output ROOT file of TCanvases", default="out.root")
    parser.add_argument("-y", help="y-axis range maximum for profile plots", default="5")
    parser.add_argument("-n", help="Comma-separated list of MMFE8s")
    parser.add_argument("-g", help="Grouping parameter for output plots", default="layer")
    parser.add_argument("-u", help="Units of measurements (mV or ENC)", default="ENC")
    return parser.parse_args()

def main():

    ops = options()
    if not ops.i:
        fatal("Please give an input txt file with -i")
    if not ops.g in ["l", "L", "layer", "Layer", "p", "P", "pcb", "PCB"]:
        fatal("Please give a grouping parameter -g like `pcb` or `layer`")
    if not ops.u in ["mV", "ENC"]:
        fatal("Please give a unit -u as mV or ENC")
    if ops.u == "ENC" and ops.y == "5":
        ops.y = "4000"


    
    rootlogon()
    out = ROOT.TFile(ops.o, "recreate")
    counter_num_lines = 0
    # parse febs from input
    febs = []
    if not ops.n:
        lines = open(ops.i).readlines()
        febs = sorted( list( set( [ line.split()[1] for line in lines ] ) ) )
    else:
        febs = ops.n.split(",")
    for feb in febs:
        print("Found %s" % (feb))
    # txt -> root
    print "Loading data..."
    tr = ROOT.TTree("DATA", "")
    tr.ReadFile(ops.i, "data/C:MMFE8:vmm_id/D:channel_id:tpdac:thdac:trim:ADCsample", " ")
    out.cd()
    tr.Write()
    print ("Data loaded.")

    ntuple = out.DATA


    out.cd()
    out.mkdir("Layer_plots")
    outdir2 = out.GetDirectory("Layer_plots")

    # debugging
    rms_of_interest = 50000.0 # mV

########  new   #########

    counter = 0
    hist_L1_HO = ROOT.TH2D("L1_HO", ";VMM*64 + Channel;Baseline [mV];Samples",8192, -0.5, 8091.5, 290, 10, 300)
    hist_L2_HO = ROOT.TH2D("L2_HO", ";VMM*64 + Channel;Baseline [mV];Samples",8192, -0.5, 8091.5, 290, 10, 300)
    hist_L3_HO = ROOT.TH2D("L3_HO", ";VMM*64 + Channel;Baseline [mV];Samples",8192, -0.5, 8091.5, 290, 10, 300)
    hist_L4_HO = ROOT.TH2D("L4_HO", ";VMM*64 + Channel;Baseline [mV];Samples",8192, -0.5, 8091.5, 290, 10, 300)
    hist_L1_IP = ROOT.TH2D("L1_IP", ";VMM*64 + Channel;Baseline [mV];Samples",8192, -0.5, 8091.5, 290, 10, 300)
    hist_L2_IP = ROOT.TH2D("L2_IP", ";VMM*64 + Channel;Baseline [mV];Samples",8192, -0.5, 8091.5, 290, 10, 300)
    hist_L3_IP = ROOT.TH2D("L3_IP", ";VMM*64 + Channel;Baseline [mV];Samples",8192, -0.5, 8091.5, 290, 10, 300)
    hist_L4_IP = ROOT.TH2D("L4_IP", ";VMM*64 + Channel;Baseline [mV];Samples",8192, -0.5, 8091.5, 290, 10, 300)
    
    counter_L1_HO = 0
    counter_L2_HO = 0
    counter_L3_HO = 0
    counter_L4_HO = 0
    counter_L1_IP = 0
    counter_L2_IP = 0
    counter_L3_IP = 0
    counter_L4_IP = 0

    
    median_board = 0;

    canvas = ROOT.TCanvas("canvas")

#########################

    # plot each feb
    for feb in febs:
        if not feb:
            continue

        
        name = "baseline_%s" % (feb)
        print "here = ", name[18], ", full name = ", name
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

        ########            #########
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
        hist_gauss.Fit(gaussFit,"S")
        hist_gauss.Draw("")
#        canvas.Print("%s_trial_gaus.pdf"%name)
#        down_limit_sigma = gaussFit.GetParameter(1) - 5*gaussFit.GetParameter(2)
#        up_limit_sigma = gaussFit.GetParameter(1) + 5*gaussFit.GetParameter(2)
        down_limit_sigma = gaussFit.GetParameter(1) - 5*hist_gauss.GetRMS()
        up_limit_sigma = gaussFit.GetParameter(1) + 5*hist_gauss.GetRMS()
        print "down = ", down_limit_sigma, ", up = ", up_limit_sigma
        print "RESULT = ",
        ########            #########

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
                if (name[16] == "3" and name[18] == "5" and name[20] == "H" and name[22] == "L") or (name[16] == "3" and name[18] == "6" and name[20] == "H" and name[22] == "R") or (name[16] == "3" and name[18] == "5" and name[20] == "I" and name[22] == "R") or (name[16] == "3" and name[18] == "6" and name[20] == "I" and name[22] == "L") or (name[16] == "3" and name[18] == "8" and name[20] == "I" and name[22] == "R") or (name[16] == "4" and name[18] == "5" and name[20] == "H" and name[22] == "R") or (name[16] == "4" and name[18] == "6" and name[20] == "H" and name[22] == "L") or (name[16] == "4" and name[18] == "8" and name[20] == "H" and name[22] == "R") or (name[16] == "4" and name[18] == "5" and name[20] == "I" and name[22] == "L") or (name[16] == "4" and name[18] == "6" and name[20] == "I" and name[22] == "R") or (name[16] == "4" and name[18] == "8" and name[20] == "I" and name[22] == "L"):
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
                    if p_sigma.GetBinContent(bin) < down_limit_sigma and p_sigma.GetBinContent(bin) != -1.5:
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



    ntuple = out.DATA
    for e in ntuple:
        name_mmfe8 = str(e.MMFE8)
        if (name_mmfe8[11] == "H" and (name_mmfe8[7] == "2" or name_mmfe8[7] == "4")) or (name_mmfe8[11] == "I" and (name_mmfe8[7] == "1" or name_mmfe8[7] == "3")):
            if name_mmfe8[13] == "R":
                ch_factor1 = 511
                ch_factor2 = 1
            elif name_mmfe8[13] == "L":
                ch_factor1 = 511
                ch_factor2 = -1
            if name_mmfe8[9] == "1" and name_mmfe8[13] == "L":
                x_input = (e.ADCsample/4095 * 1.5 * 1000)
                y_input = ch_factor1 + ch_factor2*(e.channel_id + 64*e.vmm_id)
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "2":
                        hist_L2_HO.Fill(y_input,x_input)
                        counter_L2_HO = counter_L2_HO + 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "4":
                        hist_L4_HO.Fill(y_input,x_input)
                        counter_L4_HO = counter_L4_HO + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "1":
                        hist_L1_IP.Fill(y_input,x_input)
                        counter_L1_IP = counter_L1_IP + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "3":
                        counter_L3_IP = counter_L3_IP + 1
                        hist_L3_IP.Fill(y_input,x_input)
                            
            elif name_mmfe8[9] == "1" and name_mmfe8[13] == "R":
                x_input = e.ADCsample/4095 * 1.5 * 1000
                y_input = ch_factor1 + ch_factor2*(e.channel_id + 64*e.vmm_id)
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "2":
                        hist_L2_HO.Fill(y_input,x_input)
                        counter_L2_HO = counter_L2_HO + 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "4":
                        hist_L4_HO.Fill(y_input,x_input)
                        counter_L4_HO = counter_L4_HO + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "1":
                        hist_L1_IP.Fill(y_input,x_input)
                        counter_L1_IP = counter_L1_IP + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "3":
                        hist_L3_IP.Fill(y_input,x_input)
                        counter_L3_IP = counter_L3_IP + 1
#                hist2.Fill(y_input,x_input)
#                print y_input, e.channel_id + 64*e.vmm_id, name_mmfe8
            elif name_mmfe8[9] == "2" and name_mmfe8[13] == "L":
                x_input = e.ADCsample/4095 * 1.5 * 1000
                y_input = ch_factor1*3 + ch_factor2*(e.channel_id + 64*e.vmm_id)
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "2":
                        hist_L2_HO.Fill(y_input,x_input)
                        counter_L2_HO = counter_L2_HO + 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "4":
                        hist_L4_HO.Fill(y_input,x_input)
                        counter_L4_HO = counter_L4_HO + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "1":
                        hist_L1_IP.Fill(y_input,x_input)
                        counter_L1_IP = counter_L1_IP + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "3":
                        hist_L3_IP.Fill(y_input,x_input)
                        counter_L3_IP = counter_L3_IP + 1
#                hist2.Fill(y_input,x_input)
#                print y_input, e.channel_id, 64*e.vmm_id,  e.channel_id + 64*e.vmm_id, name_mmfe8
            elif name_mmfe8[9] == "2" and name_mmfe8[13] == "R":
                x_input = e.ADCsample/4095 * 1.5 * 1000
                y_input = ch_factor1*3 + ch_factor2*(e.channel_id + 64*e.vmm_id)
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "2":
                        hist_L2_HO.Fill(y_input,x_input)
                        counter_L2_HO = counter_L2_HO + 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "4":
                        hist_L4_HO.Fill(y_input,x_input)
                        counter_L4_HO = counter_L4_HO + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "1":
                        hist_L1_IP.Fill(y_input,x_input)
                        counter_L1_IP = counter_L1_IP + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "3":
                        hist_L3_IP.Fill(y_input,x_input)
                        counter_L3_IP = counter_L3_IP + 1
#                hist2.Fill(y_input,x_input)
#                print y_input, e.channel_id, 64*e.vmm_id,  e.channel_id + 64*e.vmm_id, name_mmfe8
            elif name_mmfe8[9] == "3" and name_mmfe8[13] == "L":
                x_input = e.ADCsample/4095 * 1.5 * 1000
                y_input = ch_factor1*5 + ch_factor2*(e.channel_id + 64*e.vmm_id)
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "2":
                        hist_L2_HO.Fill(y_input,x_input)
                        counter_L2_HO = counter_L2_HO + 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "4":
                        hist_L4_HO.Fill(y_input,x_input)
                        counter_L4_HO = counter_L4_HO + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "1":
                        hist_L1_IP.Fill(y_input,x_input)
                        counter_L1_IP = counter_L1_IP + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "3":
                        hist_L3_IP.Fill(y_input,x_input)
                        counter_L3_IP = counter_L3_IP + 1
#                hist2.Fill(y_input,x_input)
#                print y_input, e.channel_id, 64*e.vmm_id,  e.channel_id + 64*e.vmm_id, name_mmfe8
            elif name_mmfe8[9] == "3" and name_mmfe8[13] == "R":
                x_input = e.ADCsample/4095 * 1.5 * 1000
                y_input = ch_factor1*5 + ch_factor2*(e.channel_id + 64*e.vmm_id)
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "2":
                        hist_L2_HO.Fill(y_input,x_input)
                        counter_L2_HO = counter_L2_HO + 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "4":
                        hist_L4_HO.Fill(y_input,x_input)
                        counter_L4_HO = counter_L4_HO + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "1":
                        hist_L1_IP.Fill(y_input,x_input)
                        counter_L1_IP = counter_L1_IP + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "3":
                        hist_L3_IP.Fill(y_input,x_input)
                        counter_L3_IP = counter_L3_IP + 1
#                hist2.Fill(y_input,x_input)
#                print y_input, e.channel_id, 64*e.vmm_id,  e.channel_id + 64*e.vmm_id, name_mmfe8
            elif name_mmfe8[9] == "4" and name_mmfe8[13] == "L":
                x_input = e.ADCsample/4095 * 1.5 * 1000
                y_input = ch_factor1*7 + ch_factor2*(e.channel_id + 64*e.vmm_id)
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "2":
                        hist_L2_HO.Fill(y_input,x_input)
                        counter_L2_HO = counter_L2_HO + 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "4":
                        hist_L4_HO.Fill(y_input,x_input)
                        counter_L4_HO = counter_L4_HO + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "1":
                        hist_L1_IP.Fill(y_input,x_input)
                        counter_L1_IP = counter_L1_IP + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "3":
                        hist_L3_IP.Fill(y_input,x_input)
                        counter_L3_IP = counter_L3_IP + 1
#                hist2.Fill(y_input,x_input)
#                print y_input, e.channel_id, 64*e.vmm_id,  e.channel_id + 64*e.vmm_id, name_mmfe8
            elif name_mmfe8[9] == "4" and name_mmfe8[13] == "R":
                x_input = e.ADCsample/4095 * 1.5 * 1000
                y_input = ch_factor1*7 + ch_factor2*(e.channel_id + 64*e.vmm_id)
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "2":
                        hist_L2_HO.Fill(y_input,x_input)
                        counter_L2_HO = counter_L2_HO + 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "4":
                        hist_L4_HO.Fill(y_input,x_input)
                        counter_L4_HO = counter_L4_HO + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "1":
                        hist_L1_IP.Fill(y_input,x_input)
                        counter_L1_IP = counter_L1_IP + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "3":
                        hist_L3_IP.Fill(y_input,x_input)
                        counter_L3_IP = counter_L3_IP + 1
#                hist2.Fill(y_input,x_input)
#                print y_input, e.channel_id, 64*e.vmm_id,  e.channel_id + 64*e.vmm_id, name_mmfe8
            elif name_mmfe8[9] == "5" and name_mmfe8[13] == "L":
                x_input = e.ADCsample/4095 * 1.5 * 1000
                y_input = ch_factor1*9 + ch_factor2*(e.channel_id + 64*e.vmm_id)
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "2":
                        hist_L2_HO.Fill(y_input,x_input)
                        counter_L2_HO = counter_L2_HO + 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "4":
                        hist_L4_HO.Fill(y_input,x_input)
                        counter_L4_HO = counter_L4_HO + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "1":
                        hist_L1_IP.Fill(y_input,x_input)
                        counter_L1_IP = counter_L1_IP + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "3":
                        hist_L3_IP.Fill(y_input,x_input)
                        counter_L3_IP = counter_L3_IP + 1
#                hist2.Fill(y_input,x_input)
#                print y_input, e.channel_id, 64*e.vmm_id,  e.channel_id + 64*e.vmm_id, name_mmfe8
            elif name_mmfe8[9] == "5" and name_mmfe8[13] == "R":
                x_input = e.ADCsample/4095 * 1.5 * 1000
                y_input = ch_factor1*9 + ch_factor2*(e.channel_id + 64*e.vmm_id)
#                print name_mmfe8
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "2":
                        hist_L2_HO.Fill(y_input,x_input)
                        counter_L2_HO = counter_L2_HO + 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "4":
                        hist_L4_HO.Fill(y_input,x_input)
                        counter_L4_HO = counter_L4_HO + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "1":
                        hist_L1_IP.Fill(y_input,x_input)
                        counter_L1_IP = counter_L1_IP + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "3":
                        hist_L3_IP.Fill(y_input,x_input)
                        counter_L3_IP = counter_L3_IP + 1
#                hist2.Fill(y_input,x_input)
#                print y_input, e.channel_id, 64*e.vmm_id,  e.channel_id + 64*e.vmm_id, name_mmfe8
            elif name_mmfe8[9] == "6" and name_mmfe8[13] == "L":
                x_input = e.ADCsample/4095 * 1.5 * 1000
                y_input = ch_factor1*11 + ch_factor2*(e.channel_id + 64*e.vmm_id)
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "2":
                        hist_L2_HO.Fill(y_input,x_input)
                        counter_L2_HO = counter_L2_HO + 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "4":
                        hist_L4_HO.Fill(y_input,x_input)
                        counter_L4_HO = counter_L4_HO + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "1":
                        hist_L1_IP.Fill(y_input,x_input)
                        counter_L1_IP = counter_L1_IP + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "3":
                        hist_L3_IP.Fill(y_input,x_input)
                        counter_L3_IP = counter_L3_IP + 1
#                hist2.Fill(y_input,x_input)
#                print y_input, e.channel_id, 64*e.vmm_id,  e.channel_id + 64*e.vmm_id, name_mmfe8
            elif name_mmfe8[9] == "6" and name_mmfe8[13] == "R":
                x_input = e.ADCsample/4095 * 1.5 * 1000
                y_input = ch_factor1*11 + ch_factor2*(e.channel_id + 64*e.vmm_id)
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "2":
                        hist_L2_HO.Fill(y_input,x_input)
                        counter_L2_HO = counter_L2_HO + 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "4":
                        hist_L4_HO.Fill(y_input,x_input)
                        counter_L4_HO = counter_L4_HO + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "1":
                        hist_L1_IP.Fill(y_input,x_input)
                        counter_L1_IP = counter_L1_IP + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "3":
                        hist_L3_IP.Fill(y_input,x_input)
                        counter_L3_IP = counter_L3_IP + 1
#                hist2.Fill(y_input,x_input)
#                print y_input, e.channel_id, 64*e.vmm_id,  e.channel_id + 64*e.vmm_id, name_mmfe8
            elif name_mmfe8[9] == "7" and name_mmfe8[13] == "L":
                x_input = e.ADCsample/4095 * 1.5 * 1000
                y_input = ch_factor1*13 + ch_factor2*(e.channel_id + 64*e.vmm_id)
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "2":
                        hist_L2_HO.Fill(y_input,x_input)
                        counter_L2_HO = counter_L2_HO + 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "4":
                        hist_L4_HO.Fill(y_input,x_input)
                        counter_L4_HO = counter_L4_HO + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "1":
                        hist_L1_IP.Fill(y_input,x_input)
                        counter_L1_IP = counter_L1_IP + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "3":
                        hist_L3_IP.Fill(y_input,x_input)
                        counter_L3_IP = counter_L3_IP + 1
#                hist2.Fill(y_input,x_input)
#                print y_input, e.channel_id, 64*e.vmm_id,  e.channel_id + 64*e.vmm_id, name_mmfe8
            elif name_mmfe8[9] == "7" and name_mmfe8[13] == "R":
                x_input = e.ADCsample/4095 * 1.5 * 1000
                y_input = ch_factor1*13 + ch_factor2*(e.channel_id + 64*e.vmm_id)
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "2":
                        hist_L2_HO.Fill(y_input,x_input)
                        counter_L2_HO = counter_L2_HO + 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "4":
                        hist_L4_HO.Fill(y_input,x_input)
                        counter_L4_HO = counter_L4_HO + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "1":
                        hist_L1_IP.Fill(y_input,x_input)
                        counter_L1_IP = counter_L1_IP + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "3":
                        hist_L3_IP.Fill(y_input,x_input)
                        counter_L3_IP = counter_L3_IP + 1
#                hist2.Fill(y_input,x_input)
#                print y_input, e.channel_id, 64*e.vmm_id,  e.channel_id + 64*e.vmm_id, name_mmfe8
            elif name_mmfe8[9] == "8" and name_mmfe8[13] == "L":
                x_input = e.ADCsample/4095 * 1.5 * 1000
                y_input = ch_factor1*15 + ch_factor2*(e.channel_id + 64*e.vmm_id)
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "2":
                        hist_L2_HO.Fill(y_input,x_input)
                        counter_L2_HO = counter_L2_HO + 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "4":
                        hist_L4_HO.Fill(y_input,x_input)
                        counter_L4_HO = counter_L4_HO + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "1":
                        hist_L1_IP.Fill(y_input,x_input)
                        counter_L1_IP = counter_L1_IP + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "3":
                        hist_L3_IP.Fill(y_input,x_input)
                        counter_L3_IP = counter_L3_IP + 1
#                hist2.Fill(y_input,x_input)
#                print y_input, e.channel_id, 64*e.vmm_id,  e.channel_id + 64*e.vmm_id, name_mmfe8
            elif name_mmfe8[9] == "8" and name_mmfe8[13] == "R":
                x_input = e.ADCsample/4095 * 1.5 * 1000
                y_input = ch_factor1*15 + ch_factor2*(e.channel_id + 64*e.vmm_id)
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "2":
                        hist_L2_HO.Fill(y_input,x_input)
                        counter_L2_HO = counter_L2_HO + 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "4":
                        hist_L4_HO.Fill(y_input,x_input)
                        counter_L4_HO = counter_L4_HO + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "1":
                        hist_L1_IP.Fill(y_input,x_input)
                        counter_L1_IP = counter_L1_IP + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "3":
                        hist_L3_IP.Fill(y_input,x_input)
                        counter_L3_IP = counter_L3_IP + 1
#                hist2.Fill(y_input,x_input)
#                print y_input, e.channel_id, 64*e.vmm_id,  e.channel_id + 64*e.vmm_id, name_mmfe8
        if (name_mmfe8[11] == "H" and (name_mmfe8[7] == "1" or name_mmfe8[7] == "3")) or (name_mmfe8[11] == "I" and (name_mmfe8[7] == "2" or name_mmfe8[7] == "4")):
            if name_mmfe8[13] == "R":
                ch_factor1 = 512
                ch_factor2 = -1
            elif name_mmfe8[13] == "L":
                ch_factor1 = 512
                ch_factor2 = 1
            if name_mmfe8[9] == "1" and name_mmfe8[13] == "L":
                x_input = (e.ADCsample/4095 * 1.5 * 1000)
                y_input = ch_factor1 + ch_factor2*(e.channel_id + 64*e.vmm_id)
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "1":
                        hist_L1_HO.Fill(y_input,x_input)
                        counter_L1_HO = counter_L1_HO + 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "3":
                        hist_L3_HO.Fill(y_input,x_input)
                        counter_L3_HO = counter_L3_HO + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "2":
                        hist_L2_IP.Fill(y_input,x_input)
                        counter_L2_IP = counter_L2_IP + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "4":
                        hist_L4_IP.Fill(y_input,x_input)
                        counter_L4_IP = counter_L4_IP + 1
#                hist2.Fill(y_input,x_input)
#                if e.channel_id + 64*e.vmm_id == 0:
#                            print y_input, e.channel_id + 64*e.vmm_id, name_mmfe8
            elif name_mmfe8[9] == "1" and name_mmfe8[13] == "R":
                x_input = e.ADCsample/4095 * 1.5 * 1000
                y_input = ch_factor1 + ch_factor2*(e.channel_id + 64*e.vmm_id) - 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "1":
                        hist_L1_HO.Fill(y_input,x_input)
                        counter_L1_HO = counter_L1_HO + 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "3":
                        hist_L3_HO.Fill(y_input,x_input)
                        counter_L3_HO = counter_L3_HO + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "2":
                        hist_L2_IP.Fill(y_input,x_input)
                        counter_L2_IP = counter_L2_IP + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "4":
                        hist_L4_IP.Fill(y_input,x_input)
                        counter_L4_IP = counter_L4_IP + 1
#                hist2.Fill(y_input,x_input)
#                if e.channel_id + 64*e.vmm_id == 511 :
#                        print y_input, e.channel_id + 64*e.vmm_id, name_mmfe8
            elif name_mmfe8[9] == "2" and name_mmfe8[13] == "L":
                x_input = e.ADCsample/4095 * 1.5 * 1000
                y_input = ch_factor1*3 + ch_factor2*(e.channel_id + 64*e.vmm_id)
#                print ch_factor1, ch_factor1*3,  e.channel_id, e.vmm_id, ch_factor1*3 + ch_factor2*(e.channel_id + 64*e.vmm_id)
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "1":
                        hist_L1_HO.Fill(y_input,x_input)
                        counter_L1_HO = counter_L1_HO + 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "3":
                        hist_L3_HO.Fill(y_input,x_input)
                        counter_L3_HO = counter_L3_HO + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "2":
                        hist_L2_IP.Fill(y_input,x_input)
                        counter_L2_IP = counter_L2_IP + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "4":
                        hist_L4_IP.Fill(y_input,x_input)
                        counter_L4_IP = counter_L4_IP + 1
#                hist2.Fill(y_input,x_input)
#                if e.channel_id + 64*e.vmm_id == 0 :
#                    print y_input, e.channel_id, 64*e.vmm_id,  e.channel_id + 64*e.vmm_id, name_mmfe8
            elif name_mmfe8[9] == "2" and name_mmfe8[13] == "R":
                x_input = e.ADCsample/4095 * 1.5 * 1000
                y_input = ch_factor1*3 + ch_factor2*(e.channel_id + 64*e.vmm_id) - 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "1":
                        hist_L1_HO.Fill(y_input,x_input)
                        counter_L1_HO = counter_L1_HO + 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "3":
                        hist_L3_HO.Fill(y_input,x_input)
                        counter_L3_HO = counter_L3_HO + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "2":
                        hist_L2_IP.Fill(y_input,x_input)
                        counter_L2_IP = counter_L2_IP + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "4":
                        hist_L4_IP.Fill(y_input,x_input)
                        counter_L4_IP = counter_L4_IP + 1
#                hist2.Fill(y_input,x_input)
#                if e.channel_id + 64*e.vmm_id == 511  :
#                    print y_input, e.channel_id, 64*e.vmm_id,  e.channel_id + 64*e.vmm_id, name_mmfe8
            elif name_mmfe8[9] == "3" and name_mmfe8[13] == "L":
                x_input = e.ADCsample/4095 * 1.5 * 1000
                y_input = ch_factor1*5 + ch_factor2*(e.channel_id + 64*e.vmm_id)
#                print ch_factor1, ch_factor1*3,  e.channel_id, e.vmm_id, ch_factor1*5 + ch_factor2*(e.channel_id + 64*e.vmm_id)
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "1":
                        hist_L1_HO.Fill(y_input,x_input)
                        counter_L1_HO = counter_L1_HO + 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "3":
                        hist_L3_HO.Fill(y_input,x_input)
                        counter_L3_HO = counter_L3_HO + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "2":
                        hist_L2_IP.Fill(y_input,x_input)
                        counter_L2_IP = counter_L2_IP + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "4":
                        hist_L4_IP.Fill(y_input,x_input)
                        counter_L4_IP = counter_L4_IP + 1
#                hist2.Fill(y_input,x_input)
#                if e.channel_id + 64*e.vmm_id == 0 :
#                    print y_input, e.channel_id, 64*e.vmm_id,  e.channel_id + 64*e.vmm_id, name_mmfe8
            elif name_mmfe8[9] == "3" and name_mmfe8[13] == "R":
                x_input = e.ADCsample/4095 * 1.5 * 1000
                y_input = ch_factor1*5 + ch_factor2*(e.channel_id + 64*e.vmm_id) - 1
#                print ch_factor1, ch_factor1*5,  e.channel_id, e.vmm_id, ch_factor1*5 + ch_factor2*(e.channel_id + 64*e.vmm_id) - 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "1":
                        hist_L1_HO.Fill(y_input,x_input)
                        counter_L1_HO = counter_L1_HO + 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "3":
                        hist_L3_HO.Fill(y_input,x_input)
                        counter_L3_HO = counter_L3_HO + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "2":
                        hist_L2_IP.Fill(y_input,x_input)
                        counter_L2_IP = counter_L2_IP + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "4":
                        hist_L4_IP.Fill(y_input,x_input)
                        counter_L4_IP = counter_L4_IP + 1
#                hist2.Fill(y_input,x_input)
#                if e.channel_id + 64*e.vmm_id == 511 :
#                    print y_input, e.channel_id, 64*e.vmm_id,  e.channel_id + 64*e.vmm_id, name_mmfe8
            elif name_mmfe8[9] == "4" and name_mmfe8[13] == "L":
                x_input = e.ADCsample/4095 * 1.5 * 1000
                y_input = ch_factor1*7 + ch_factor2*(e.channel_id + 64*e.vmm_id)
#                print ch_factor1, ch_factor1*7,  e.channel_id, e.vmm_id, ch_factor1*7 + ch_factor2*(e.channel_id + 64*e.vmm_id)
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "1":
                        hist_L1_HO.Fill(y_input,x_input)
                        counter_L1_HO = counter_L1_HO + 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "3":
                        hist_L3_HO.Fill(y_input,x_input)
                        counter_L3_HO = counter_L3_HO + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "2":
                        hist_L2_IP.Fill(y_input,x_input)
                        counter_L2_IP = counter_L2_IP + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "4":
                        hist_L4_IP.Fill(y_input,x_input)
                        counter_L4_IP = counter_L4_IP + 1
#                hist2.Fill(y_input,x_input)
#                if e.channel_id + 64*e.vmm_id == 0 :
#                    print y_input, e.channel_id, 64*e.vmm_id,  e.channel_id + 64*e.vmm_id, name_mmfe8
            elif name_mmfe8[9] == "4" and name_mmfe8[13] == "R":
                x_input = e.ADCsample/4095 * 1.5 * 1000
                y_input = ch_factor1*7 + ch_factor2*(e.channel_id + 64*e.vmm_id) - 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "1":
                        hist_L1_HO.Fill(y_input,x_input)
                        counter_L1_HO = counter_L1_HO + 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "3":
                        hist_L3_HO.Fill(y_input,x_input)
                        counter_L3_HO = counter_L3_HO + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "2":
                        hist_L2_IP.Fill(y_input,x_input)
                        counter_L2_IP = counter_L2_IP + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "4":
                        hist_L4_IP.Fill(y_input,x_input)
                        counter_L4_IP = counter_L4_IP + 1
#                if e.channel_id + 64*e.vmm_id == 511 :
#                hist2.Fill(y_input,x_input)
#                    print y_input, e.channel_id, 64*e.vmm_id,  e.channel_id + 64*e.vmm_id, name_mmfe8
            elif name_mmfe8[9] == "5" and name_mmfe8[13] == "L":
                x_input = e.ADCsample/4095 * 1.5 * 1000
                y_input = ch_factor1*9 + ch_factor2*(e.channel_id + 64*e.vmm_id)
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "1":
                        hist_L1_HO.Fill(y_input,x_input)
                        counter_L1_HO = counter_L1_HO + 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "3":
                        hist_L3_HO.Fill(y_input,x_input)
                        counter_L3_HO = counter_L3_HO + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "2":
                        hist_L2_IP.Fill(y_input,x_input)
                        counter_L2_IP = counter_L2_IP + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "4":
                        hist_L4_IP.Fill(y_input,x_input)
                        counter_L4_IP = counter_L4_IP + 1
                        print name_mmfe8
#                if e.channel_id + 64*e.vmm_id == 0 :
#                hist2.Fill(y_input,x_input)
#                    print y_input, e.channel_id, 64*e.vmm_id,  e.channel_id + 64*e.vmm_id, name_mmfe8
            elif name_mmfe8[9] == "5" and name_mmfe8[13] == "R":
                x_input = e.ADCsample/4095 * 1.5 * 1000
                y_input = ch_factor1*9 + ch_factor2*(e.channel_id + 64*e.vmm_id) - 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "1":
                        hist_L1_HO.Fill(y_input,x_input)
                        counter_L1_HO = counter_L1_HO + 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "3":
                        hist_L3_HO.Fill(y_input,x_input)
                        counter_L3_HO = counter_L3_HO + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "2":
                        hist_L2_IP.Fill(y_input,x_input)
                        counter_L2_IP = counter_L2_IP + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "4":
                        hist_L4_IP.Fill(y_input,x_input)
                        counter_L4_IP = counter_L4_IP + 1
                        print name_mmfe8
#                if e.channel_id + 64*e.vmm_id == 511 :
#                        print y_input, e.channel_id, 64*e.vmm_id,  e.channel_id + 64*e.vmm_id, name_mmfe8
            elif name_mmfe8[9] == "6" and name_mmfe8[13] == "L":
                x_input = e.ADCsample/4095 * 1.5 * 1000
                y_input = ch_factor1*11 + ch_factor2*(e.channel_id + 64*e.vmm_id)
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "1":
                        hist_L1_HO.Fill(y_input,x_input)
                        counter_L1_HO = counter_L1_HO + 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "3":
                        hist_L3_HO.Fill(y_input,x_input)
                        counter_L3_HO = counter_L3_HO + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "2":
                        hist_L2_IP.Fill(y_input,x_input)
                        counter_L2_IP = counter_L2_IP + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "4":
                        hist_L4_IP.Fill(y_input,x_input)
                        counter_L4_IP = counter_L4_IP + 1
#                if e.channel_id + 64*e.vmm_id == 0 :
#                hist2.Fill(y_input,x_input)
#                    print y_input, e.channel_id, 64*e.vmm_id,  e.channel_id + 64*e.vmm_id, name_mmfe8
            elif name_mmfe8[9] == "6" and name_mmfe8[13] == "R":
                x_input = e.ADCsample/4095 * 1.5 * 1000
                y_input = ch_factor1*11 + ch_factor2*(e.channel_id + 64*e.vmm_id) - 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "1":
                        hist_L1_HO.Fill(y_input,x_input)
                        counter_L1_HO = counter_L1_HO + 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "3":
                        hist_L3_HO.Fill(y_input,x_input)
                        counter_L3_HO = counter_L3_HO + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "2":
                        hist_L2_IP.Fill(y_input,x_input)
                        counter_L2_IP = counter_L2_IP + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "4":
                        hist_L4_IP.Fill(y_input,x_input)
                        counter_L4_IP = counter_L4_IP + 1
#                if e.channel_id + 64*e.vmm_id == 511 :
#                    print y_input, e.channel_id, 64*e.vmm_id,  e.channel_id + 64*e.vmm_id, name_mmfe8
            elif name_mmfe8[9] == "7" and name_mmfe8[13] == "L":
                x_input = e.ADCsample/4095 * 1.5 * 1000
                y_input = ch_factor1*13 + ch_factor2*(e.channel_id + 64*e.vmm_id)
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "1":
                        hist_L1_HO.Fill(y_input,x_input)
                        counter_L1_HO = counter_L1_HO + 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "3":
                        hist_L3_HO.Fill(y_input,x_input)
                        counter_L3_HO = counter_L3_HO + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "2":
                        hist_L2_IP.Fill(y_input,x_input)
                        counter_L2_IP = counter_L2_IP + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "4":
                        hist_L4_IP.Fill(y_input,x_input)
                        counter_L4_IP = counter_L4_IP + 1
#                if e.channel_id + 64*e.vmm_id == 0 :
#                hist2.Fill(y_input,x_input)
#                    print y_input, e.channel_id, 64*e.vmm_id,  e.channel_id + 64*e.vmm_id, name_mmfe8
            elif name_mmfe8[9] == "7" and name_mmfe8[13] == "R":
                x_input = e.ADCsample/4095 * 1.5 * 1000
                y_input = ch_factor1*13 + ch_factor2*(e.channel_id + 64*e.vmm_id) - 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "1":
                        hist_L1_HO.Fill(y_input,x_input)
                        counter_L1_HO = counter_L1_HO + 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "3":
                        hist_L3_HO.Fill(y_input,x_input)
                        counter_L3_HO = counter_L3_HO + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "2":
                        hist_L2_IP.Fill(y_input,x_input)
                        counter_L2_IP = counter_L2_IP + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "4":
                        hist_L4_IP.Fill(y_input,x_input)
                        counter_L4_IP = counter_L4_IP + 1
#                if e.channel_id + 64*e.vmm_id == 511 :
#                hist2.Fill(y_input,x_input)
#                    print y_input, e.channel_id, 64*e.vmm_id,  e.channel_id + 64*e.vmm_id, name_mmfe8
            elif name_mmfe8[9] == "8" and name_mmfe8[13] == "L":
                x_input = e.ADCsample/4095 * 1.5 * 1000
                y_input = ch_factor1*15 + ch_factor2*(e.channel_id + 64*e.vmm_id)
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "1":
                        hist_L1_HO.Fill(y_input,x_input)
                        counter_L1_HO = counter_L1_HO + 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "3":
                        hist_L3_HO.Fill(y_input,x_input)
                        counter_L3_HO = counter_L3_HO + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "2":
                        hist_L2_IP.Fill(y_input,x_input)
                        counter_L2_IP = counter_L2_IP + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "4":
                        hist_L4_IP.Fill(y_input,x_input)
                        counter_L4_IP = counter_L4_IP + 1
#                if e.channel_id + 64*e.vmm_id == 0 :
#                hist2.Fill(y_input,x_input)
#                    print y_input, e.channel_id, 64*e.vmm_id,  e.channel_id + 64*e.vmm_id, name_mmfe8
            elif name_mmfe8[9] == "8" and name_mmfe8[13] == "R":
                x_input = e.ADCsample/4095 * 1.5 * 1000
                y_input = ch_factor1*15 + ch_factor2*(e.channel_id + 64*e.vmm_id) - 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "1":
                        hist_L1_HO.Fill(y_input,x_input)
                        counter_L1_HO = counter_L1_HO + 1
                if name_mmfe8[11] == "H" and name_mmfe8[7] == "3":
                        hist_L3_HO.Fill(y_input,x_input)
                        counter_L3_HO = counter_L3_HO + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "2":
                        hist_L2_IP.Fill(y_input,x_input)
                        counter_L2_IP = counter_L2_IP + 1
                if name_mmfe8[11] == "I" and name_mmfe8[7] == "4":
                        hist_L4_IP.Fill(y_input,x_input)
                        counter_L4_IP = counter_L4_IP + 1
#                if e.channel_id + 64*e.vmm_id == 511 :
#                hist2.Fill(y_input,x_input)
#                    print y_input, e.channel_id, 64*e.vmm_id,  e.channel_id + 64*e.vmm_id, name_mmfe8

#    print counter_L1_HO, counter_L2_HO, counter_L3_HO, counter_L4_HO, counter_L1_IP, counter_L2_IP, counter_L3_IP, counter_L4_IP
    if counter_L1_HO != 0:
        style(hist_L2_HO)
        can_L1_HO = ROOT.TCanvas("baseline_L1_HO", "baseline_L1_HO", 800, 800)
        can_L1_HO.cd()
        can_L1_HO.Draw()
        hist_L1_HO.Draw("colzsame")
        outdir2.cd()
        can_L1_HO.Write()
    if counter_L2_HO != 0:
        style(hist_L2_HO)
        can_L2_HO = ROOT.TCanvas("baseline_L2_HO", "baseline_L2_HO", 800, 800)
        can_L2_HO.cd()
        can_L2_HO.Draw()
        hist_L2_HO.Draw("colzsame")
        outdir2.cd()
        can_L2_HO.Write()
    if counter_L3_HO != 0:
        style(hist_L3_HO)
        can_L3_HO = ROOT.TCanvas("baseline_L3_HO", "baseline_L3_HO", 800, 800)
        can_L3_HO.cd()
        can_L3_HO.Draw()
        hist_L3_HO.Draw("colzsame")
        outdir2.cd()
        can_L3_HO.Write()
    if counter_L4_HO != 0:
        style(hist_L4_HO)
        can_L4_HO = ROOT.TCanvas("baseline_L4_HO", "baseline_L4_HO", 800, 800)
        can_L4_HO.cd()
        can_L4_HO.Draw()
        hist_L4_HO.Draw("colzsame")
        outdir2.cd()
        can_L4_HO.Write()
    if counter_L1_IP != 0:
        style(hist_L1_IP)
        can_L1_IP = ROOT.TCanvas("baseline_L1_IP", "baseline_L1_IP", 800, 800)
        can_L1_IP.cd()
        can_L1_IP.Draw()
        hist_L1_IP.Draw("colzsame")
        outdir2.cd()
        can_L1_IP.Write()
    if counter_L2_IP != 0:
        style(hist_L2_IP)
        can_L2_IP = ROOT.TCanvas("baseline_L2_IP", "baseline_L2_IP", 800, 800)
        can_L2_IP.cd()
        can_L2_IP.Draw()
        hist_L2_IP.Draw("colzsame")
        outdir2.cd()
        can_L2_IP.Write()
    if counter_L3_IP != 0:
        style(hist_L3_IP)
        can_L3_IP = ROOT.TCanvas("baseline_L3_IP", "baseline_L3_IP", 800, 800)
        can_L3_IP.cd()
        can_L3_IP.Draw()
        hist_L3_IP.Draw("colzsame")
        outdir2.cd()
        can_L3_IP.Write()
    if counter_L4_IP != 0:
        style(hist_L4_IP)
        can_L4_IP = ROOT.TCanvas("baseline_L4_IP", "baseline_L4_IP", 800, 800)
        can_L4_IP.cd()
        can_L4_IP.Draw()
        hist_L4_IP.Draw("colzsame")
        outdir2.cd()
        can_L4_IP.Write()



########  RMS plot ########

    lines2 = []
    for mmfe8_lines in range(1, 16):
        x = mmfe8_lines*512
        lines2.append( ROOT.TLine(x, 0, x, float(ops.y)) )
        lines2[-1].SetLineColor(17)
        lines2[-1].SetLineStyle(7)

    if counter_L1_HO != 0:
        tmpx_L1_HO = hist_L1_HO.ProfileX("_tmpX_L1_HO", 1, hist_L1_HO.GetNbinsY(), "s")
        prof_L1_HO = ROOT.TH1D("L1_HO_prof", ";VMM*64 + Channel;Baseline RMS [%s]" % (ops.u), 8192, -0.5, 8191.5)
        for bin in range(0, tmpx_L1_HO.GetNbinsX()+1):
            multiplier = 1.0 if ops.u=="mV" else 6241.50975/9.0
            prof_L1_HO.SetBinContent(bin, tmpx_L1_HO.GetBinError(bin) * multiplier)
            prof_L1_HO.SetBinError  (bin, 0)
            if prof_L1_HO.GetBinContent(bin) > rms_of_interest:
                    # bin 1 is channel 0
                    print_me = "%s Bin %03i has RMS %4.1f => VMM/CH = %i/%02i"
    #                print print_me % (feb, (bin-1), prof.GetBinContent(bin), (bin-1)/64, (bin-1)%64)
    # print "%s Bin %03i has RMS %4.1f => VMM/CH = %i/%02i"
    # % (feb, (bin-1), prof.GetBinContent(bin), (bin-1)/64, (bin-1)%64)
    #if prof.GetBinContent(bin) > float(ops.y):
    #    prof.SetBinContent(bin, float(ops.y)-0.1)
        style(prof_L1_HO)
        prof_L1_HO.SetTitleSize(0.5)
#        prof_L1_HO.SetTitle("L1_HO")
        prof_L1_HO.SetMinimum(0)
        prof_L1_HO.SetMaximum(float(ops.y))
        prof_L1_HO.SetLineWidth(0)
        prof_L1_HO.SetMarkerStyle(ROOT.kFullCircle)
        prof_L1_HO.SetMarkerSize(0.2)
        prof_L1_HO.SetMarkerColor(ROOT.kBlack)

# VMM mean/median
        latexs = []
#        for vmm in range(0, 8):
        for mmfe8_latex in range(0, 16):
            x = mmfe8_latex*512
            mean   = prof_L1_HO.Integral(x+1, x+512) / 512.0
            median = sorted([prof_L1_HO.GetBinContent(bin) for bin in xrange(x+1, x+512+1)]) [512/2]
            val    = median # change me at your desire
            string = ("%.2f" % (val)) if ops.u=="mV" else str(int(val))
            latexs.append( ROOT.TLatex(x+512/2, float(ops.y)*1.02, string) )
            latexs[-1].SetTextSize(0.025)
            latexs[-1].SetTextFont(42)
            latexs[-1].SetTextAlign(21)
            lines.append( ROOT.TLine(x, val, x+64, val) )
            lines[-1].SetLineColor(ROOT.kBlack)
            lines[-1].SetLineStyle(1)
            lines[-1].SetLineWidth(1)
        
        
            # clone for overflow
            p_up_L1_HO = prof_L1_HO.Clone(prof_L1_HO.GetName()+"_clone_up")
            for bin in range(0, p_up_L1_HO.GetNbinsX()+1):
                if p_up_L1_HO.GetBinContent(bin) > float(ops.y):
                    p_up_L1_HO.SetBinContent(bin, float(ops.y)-0.01)
                else:
                    p_up_L1_HO.SetBinContent(bin, -1)
            style(p_up_L1_HO)
            p_up_L1_HO.SetMinimum(0)
            p_up_L1_HO.SetMaximum(float(ops.y))
            p_up_L1_HO.SetMarkerStyle(ROOT.kFullTriangleUp)
            p_up_L1_HO.SetMarkerColor(ROOT.kRed)
            p_up_L1_HO.SetMarkerSize(1.0)
        
        # clone for underflow
            p_dn_L1_HO = prof_L1_HO.Clone(prof_L1_HO.GetName()+"_clone_dn")
            for bin in range(0, p_dn_L1_HO.GetNbinsX()+1):
                if p_dn_L1_HO.GetBinContent(bin) < 0.1:
                    p_dn_L1_HO.SetBinContent(bin, 0.01)
                else:
                    p_dn_L1_HO.SetBinContent(bin, -1)
            style(p_dn_L1_HO)
            p_dn_L1_HO.SetMinimum(0)
            p_dn_L1_HO.SetMaximum(float(ops.y))
            p_dn_L1_HO.SetMarkerStyle(ROOT.kFullCircle)
            p_dn_L1_HO.SetMarkerColor(ROOT.kRed)
        # draw
        canv3 = ROOT.TCanvas("L1_HO_prfx", "L1_HO_prfx", 1600, 800)
        canv3.Draw()
        p_up_L1_HO.Draw("psame")
        p_dn_L1_HO.Draw("psame")
        for line in lines2:
            line.Draw()
        for latex in latexs:
            latex.Draw()
        prof_L1_HO.Draw("psame")
        outdir2.cd()
#        febdir.cd()
        canv3.Write()
        canv3.Close()

    if counter_L2_HO != 0:
        tmpx_L2_HO = hist_L2_HO.ProfileX("_tmpX_L2_HO", 1, hist_L2_HO.GetNbinsY(), "s")
        prof_L2_HO = ROOT.TH1D("L2_HO_prof", ";VMM*64 + Channel;Baseline RMS [%s]" % (ops.u), 8192, -0.5, 8191.5)
        for bin in range(0, tmpx_L2_HO.GetNbinsX()+1):
            multiplier = 1.0 if ops.u=="mV" else 6241.50975/9.0
            prof_L2_HO.SetBinContent(bin, tmpx_L2_HO.GetBinError(bin) * multiplier)
            prof_L2_HO.SetBinError  (bin, 0)
            if prof_L2_HO.GetBinContent(bin) > rms_of_interest:
                # bin 1 is channel 0
                print_me = "%s Bin %03i has RMS %4.1f => VMM/CH = %i/%02i"
    
        style(prof_L2_HO)
        prof_L2_HO.SetTitleSize(0.5)
#        prof_L2_HO.SetTitle("L2_HO")
        prof_L2_HO.SetMinimum(0)
        prof_L2_HO.SetMaximum(float(ops.y))
        prof_L2_HO.SetLineWidth(0)
        prof_L2_HO.SetMarkerStyle(ROOT.kFullCircle)
        prof_L2_HO.SetMarkerSize(0.2)
        prof_L2_HO.SetMarkerColor(ROOT.kBlack)
        
        # VMM mean/median
        latexs = []
        for mmfe8_latex in range(0, 16):
            x = mmfe8_latex*512
            mean   = prof_L2_HO.Integral(x+1, x+512) / 512.0
            median = sorted([prof_L2_HO.GetBinContent(bin) for bin in xrange(x+1, x+512+1)]) [512/2]
            val    = median # change me at your desire
            string = ("%.2f" % (val)) if ops.u=="mV" else str(int(val))
            latexs.append( ROOT.TLatex(x+512/2, float(ops.y)*1.02, string) )
            latexs[-1].SetTextSize(0.025)
            latexs[-1].SetTextFont(42)
            latexs[-1].SetTextAlign(21)
            lines.append( ROOT.TLine(x, val, x+64, val) )
            lines[-1].SetLineColor(ROOT.kBlack)
            lines[-1].SetLineStyle(1)
            lines[-1].SetLineWidth(1)
            
            
            # clone for overflow
            p_up_L2_HO = prof_L2_HO.Clone(prof_L2_HO.GetName()+"_clone_up")
            for bin in range(0, p_up_L2_HO.GetNbinsX()+1):
                if p_up_L2_HO.GetBinContent(bin) > float(ops.y):
                    p_up_L2_HO.SetBinContent(bin, float(ops.y)-0.01)
                else:
                    p_up_L2_HO.SetBinContent(bin, -1)
            style(p_up_L2_HO)
            p_up_L2_HO.SetMinimum(0)
            p_up_L2_HO.SetMaximum(float(ops.y))
            p_up_L2_HO.SetMarkerStyle(ROOT.kFullTriangleUp)
            p_up_L2_HO.SetMarkerColor(ROOT.kRed)
            p_up_L2_HO.SetMarkerSize(1.0)
            
            # clone for underflow
            p_dn_L2_HO = prof_L2_HO.Clone(prof_L2_HO.GetName()+"_clone_dn")
            for bin in range(0, p_dn_L2_HO.GetNbinsX()+1):
                if p_dn_L2_HO.GetBinContent(bin) < 0.1:
                    p_dn_L2_HO.SetBinContent(bin, 0.01)
                else:
                    p_dn_L2_HO.SetBinContent(bin, -1)
            style(p_dn_L2_HO)
            p_dn_L2_HO.SetMinimum(0)
            p_dn_L2_HO.SetMaximum(float(ops.y))
            p_dn_L2_HO.SetMarkerStyle(ROOT.kFullCircle)
            p_dn_L2_HO.SetMarkerColor(ROOT.kRed)
        # draw
        canv3 = ROOT.TCanvas("L2_HO_prfx", "L2_HO_prfx", 1600, 800)
        canv3.Draw()
        p_up_L2_HO.Draw("psame")
        p_dn_L2_HO.Draw("psame")
        for line in lines2:
            line.Draw()
        for latex in latexs:
            latex.Draw()
        prof_L2_HO.Draw("psame")
        outdir2.cd()
#        febdir.cd()
        canv3.Write()
        canv3.Close()



    if counter_L3_HO != 0:
        tmpx_L3_HO = hist_L3_HO.ProfileX("_tmpX_L3_HO", 1, hist_L3_HO.GetNbinsY(), "s")
        prof_L3_HO = ROOT.TH1D("L3_HO_prof", ";VMM*64 + Channel;Baseline RMS [%s]" % (ops.u), 8192, -0.5, 8191.5)
        for bin in range(0, tmpx_L3_HO.GetNbinsX()+1):
            multiplier = 1.0 if ops.u=="mV" else 6241.50975/9.0
            prof_L3_HO.SetBinContent(bin, tmpx_L3_HO.GetBinError(bin) * multiplier)
            prof_L3_HO.SetBinError  (bin, 0)
            if prof_L3_HO.GetBinContent(bin) > rms_of_interest:
                    # bin 1 is channel 0
                    print_me = "%s Bin %03i has RMS %4.1f => VMM/CH = %i/%02i"

        style(prof_L3_HO)
        prof_L3_HO.SetTitleSize(0.5)
#        prof_L3_HO.SetTitle("L3_HO")
        prof_L3_HO.SetMinimum(0)
        prof_L3_HO.SetMaximum(float(ops.y))
        prof_L3_HO.SetLineWidth(0)
        prof_L3_HO.SetMarkerStyle(ROOT.kFullCircle)
        prof_L3_HO.SetMarkerSize(0.2)
        prof_L3_HO.SetMarkerColor(ROOT.kBlack)

# VMM mean/median
        latexs2 = []
        for mmfe8_latex in range(0, 16):
            x = mmfe8_latex*512
            mean   = prof_L3_HO.Integral(x+1, x+512) / 512.0
            median = sorted([prof_L3_HO.GetBinContent(bin) for bin in xrange(x+1, x+512+1)]) [512/2]
            val    = median # change me at your desire
            string = ("%.2f" % (val)) if ops.u=="mV" else str(int(val))
            latexs2.append( ROOT.TLatex(x+512/2, float(ops.y)*1.02, string) )
            latexs2[-1].SetTextSize(0.015)
            latexs2[-1].SetTextFont(42)
            latexs2[-1].SetTextAlign(21)
            lines2.append( ROOT.TLine(x, val, x+512, val) )
            lines2[-1].SetLineColor(ROOT.kBlack)
            lines2[-1].SetLineStyle(1)
            lines2[-1].SetLineWidth(1)
            
            
            # clone for overflow
#            print "HERE", ops.y
            p_up_L3_HO = prof_L3_HO.Clone(prof_L3_HO.GetName()+"_clone_up")
            for bin in range(0, p_up_L3_HO.GetNbinsX()+1):
                if p_up_L3_HO.GetBinContent(bin) > float(ops.y):
                    p_up_L3_HO.SetBinContent(bin, float(ops.y)-0.01)
                else:
                    p_up_L3_HO.SetBinContent(bin, -1)
            style(p_up_L3_HO)
            p_up_L3_HO.SetMinimum(0)
            p_up_L3_HO.SetMaximum(float(ops.y))
            p_up_L3_HO.SetMarkerStyle(ROOT.kFullTriangleUp)
            p_up_L3_HO.SetMarkerColor(ROOT.kRed)
            p_up_L3_HO.SetMarkerSize(1.0)
            
            # clone for underflow
            p_dn_L3_HO = prof_L3_HO.Clone(prof_L3_HO.GetName()+"_clone_dn")
            for bin in range(0, p_dn_L3_HO.GetNbinsX()+1):
                if p_dn_L3_HO.GetBinContent(bin) < 0.1:
                    if p_dn_L3_HO.GetBinContent(bin) != float(0):
#                        print bin, p_dn_L3_HO.GetBinContent(bin)
                        p_dn_L3_HO.SetBinContent(bin, 0.01)
                else:
                    p_dn_L3_HO.SetBinContent(bin, -1)
            style(p_dn_L3_HO)
            p_dn_L3_HO.SetMinimum(0)
            p_dn_L3_HO.SetMaximum(float(ops.y))
            p_dn_L3_HO.SetMarkerStyle(ROOT.kFullCircle)
            p_dn_L3_HO.SetMarkerColor(ROOT.kRed)
        # draw
        canv3 = ROOT.TCanvas("L3_HO_prfx", "L3_HO_prfx", 1600, 800)
        canv3.Draw()
        p_up_L3_HO.Draw("psame")
        p_dn_L3_HO.Draw("psame")
        counter_lines = 1
        for line in lines2:
            line.Draw()
            counter_lines = counter_lines +1
        for latex in latexs2:
            latex.Draw()
        prof_L3_HO.Draw("psame")
        outdir2.cd()
#        febdir.cd()
        canv3.Write()
        canv3.Close()
        


    if counter_L4_HO != 0:
        tmpx_L4_HO = hist_L4_HO.ProfileX("_tmpX_L4_HO", 1, hist_L4_HO.GetNbinsY(), "s")
        prof_L4_HO = ROOT.TH1D("L4_HO_prof", ";VMM*64 + Channel;Baseline RMS [%s]" % (ops.u), 8192, -0.5, 8191.5)
        for bin in range(0, tmpx_L4_HO.GetNbinsX()+1):
            multiplier = 1.0 if ops.u=="mV" else 6241.50975/9.0
            prof_L4_HO.SetBinContent(bin, tmpx_L4_HO.GetBinError(bin) * multiplier)
            prof_L4_HO.SetBinError  (bin, 0)
            if prof_L4_HO.GetBinContent(bin) > rms_of_interest:
                # bin 1 is channel 0
                print_me = "%s Bin %03i has RMS %4.1f => VMM/CH = %i/%02i"
    
        style(prof_L4_HO)
        prof_L4_HO.SetTitleSize(0.5)
#        prof_L4_HO.SetTitle("L4_HO")
        prof_L4_HO.SetMinimum(0)
        prof_L4_HO.SetMaximum(float(ops.y))
        prof_L4_HO.SetLineWidth(0)
        prof_L4_HO.SetMarkerStyle(ROOT.kFullCircle)
        prof_L4_HO.SetMarkerSize(0.2)
        prof_L4_HO.SetMarkerColor(ROOT.kBlack)
        
        # VMM mean/median
        latexs = []
        for mmfe8_latex in range(0, 16):
            x = mmfe8_latex*512
            mean   = prof_L4_HO.Integral(x+1, x+512) / 512.0
            median = sorted([prof_L4_HO.GetBinContent(bin) for bin in xrange(x+1, x+512+1)]) [512/2]
            val    = median # change me at your desire
            string = ("%.2f" % (val)) if ops.u=="mV" else str(int(val))
            latexs.append( ROOT.TLatex(x+512/2, float(ops.y)*1.02, string) )
            latexs[-1].SetTextSize(0.025)
            latexs[-1].SetTextFont(42)
            latexs[-1].SetTextAlign(21)
            lines.append( ROOT.TLine(x, val, x+64, val) )
            lines[-1].SetLineColor(ROOT.kBlack)
            lines[-1].SetLineStyle(1)
            lines[-1].SetLineWidth(1)
            
            
            # clone for overflow
            p_up_L4_HO = prof_L4_HO.Clone(prof_L4_HO.GetName()+"_clone_up")
            for bin in range(0, p_up_L4_HO.GetNbinsX()+1):
                if p_up_L4_HO.GetBinContent(bin) > float(ops.y):
                    p_up_L4_HO.SetBinContent(bin, float(ops.y)-0.01)
                else:
                    p_up_L4_HO.SetBinContent(bin, -1)
            style(p_up_L4_HO)
            p_up_L4_HO.SetMinimum(0)
            p_up_L4_HO.SetMaximum(float(ops.y))
            p_up_L4_HO.SetMarkerStyle(ROOT.kFullTriangleUp)
            p_up_L4_HO.SetMarkerColor(ROOT.kRed)
            p_up_L4_HO.SetMarkerSize(1.0)
            
            # clone for underflow
            p_dn_L4_HO = prof_L4_HO.Clone(prof_L4_HO.GetName()+"_clone_dn")
            for bin in range(0, p_dn_L4_HO.GetNbinsX()+1):
                if p_dn_L4_HO.GetBinContent(bin) < 0.1:
                    p_dn_L4_HO.SetBinContent(bin, 0.01)
                else:
                    p_dn_L4_HO.SetBinContent(bin, -1)
            style(p_dn_L4_HO)
            p_dn_L4_HO.SetMinimum(0)
            p_dn_L4_HO.SetMaximum(float(ops.y))
            p_dn_L4_HO.SetMarkerStyle(ROOT.kFullCircle)
            p_dn_L4_HO.SetMarkerColor(ROOT.kRed)
        # draw
        canv3 = ROOT.TCanvas("L4_HO_prfx", "L4_HO_prfx", 1600, 800)
        canv3.Draw()
        p_up_L4_HO.Draw("psame")
        p_dn_L4_HO.Draw("psame")
        for line in lines2:
            line.Draw()
        for latex in latexs:
            latex.Draw()
        prof_L4_HO.Draw("psame")
        outdir2.cd()
        canv3.Write()
        canv3.Close()
            
            


    if counter_L1_IP != 0:
        tmpx_L1_IP = hist_L1_IP.ProfileX("_tmpX_L1_IP", 1, hist_L1_IP.GetNbinsY(), "s")
        prof_L1_IP = ROOT.TH1D("L1_IP_prof", ";VMM*64 + Channel;Baseline RMS [%s]" % (ops.u), 8192, -0.5, 8191.5)
        for bin in range(0, tmpx_L1_IP.GetNbinsX()+1):
            multiplier = 1.0 if ops.u=="mV" else 6241.50975/9.0
            prof_L1_IP.SetBinContent(bin, tmpx_L1_IP.GetBinError(bin) * multiplier)
            prof_L1_IP.SetBinError  (bin, 0)
            if prof_L1_IP.GetBinContent(bin) > rms_of_interest:
                # bin 1 is channel 0
                print_me = "%s Bin %03i has RMS %4.1f => VMM/CH = %i/%02i"

        style(prof_L1_IP)
        prof_L1_IP.SetTitleSize(0.5)
#        prof_L1_IP.SetTitle("L1_IP")
        prof_L1_IP.SetMinimum(0)
        prof_L1_IP.SetMaximum(float(ops.y))
        prof_L1_IP.SetLineWidth(0)
        prof_L1_IP.SetMarkerStyle(ROOT.kFullCircle)
        prof_L1_IP.SetMarkerSize(0.2)
        prof_L1_IP.SetMarkerColor(ROOT.kBlack)

    # VMM mean/median
        latexs = []
        for mmfe8_latex in range(0, 16):
            x = mmfe8_latex*512
            mean   = prof_L1_IP.Integral(x+1, x+512) / 512.0
            median = sorted([prof_L1_IP.GetBinContent(bin) for bin in xrange(x+1, x+512+1)]) [512/2]
            val    = median # change me at your desire
            string = ("%.2f" % (val)) if ops.u=="mV" else str(int(val))
            latexs.append( ROOT.TLatex(x+512/2, float(ops.y)*1.02, string) )
            latexs[-1].SetTextSize(0.025)
            latexs[-1].SetTextFont(42)
            latexs[-1].SetTextAlign(21)
            lines.append( ROOT.TLine(x, val, x+64, val) )
            lines[-1].SetLineColor(ROOT.kBlack)
            lines[-1].SetLineStyle(1)
            lines[-1].SetLineWidth(1)
            
            
            # clone for overflow
            p_up_L1_IP = prof_L1_IP.Clone(prof_L1_IP.GetName()+"_clone_up")
            for bin in range(0, p_up_L1_IP.GetNbinsX()+1):
                if p_up_L1_IP.GetBinContent(bin) > float(ops.y):
                    p_up_L1_IP.SetBinContent(bin, float(ops.y)-0.01)
                else:
                    p_up_L1_IP.SetBinContent(bin, -1)
            style(p_up_L1_IP)
            p_up_L1_IP.SetMinimum(0)
            p_up_L1_IP.SetMaximum(float(ops.y))
            p_up_L1_IP.SetMarkerStyle(ROOT.kFullTriangleUp)
            p_up_L1_IP.SetMarkerColor(ROOT.kRed)
            p_up_L1_IP.SetMarkerSize(1.0)
            
            # clone for underflow
            p_dn_L1_IP = prof_L1_IP.Clone(prof_L1_IP.GetName()+"_clone_dn")
            for bin in range(0, p_dn_L1_IP.GetNbinsX()+1):
                if p_dn_L1_IP.GetBinContent(bin) < 0.1:
                    p_dn_L1_IP.SetBinContent(bin, 0.01)
                else:
                    p_dn_L1_IP.SetBinContent(bin, -1)
            style(p_dn_L1_IP)
            p_dn_L1_IP.SetMinimum(0)
            p_dn_L1_IP.SetMaximum(float(ops.y))
            p_dn_L1_IP.SetMarkerStyle(ROOT.kFullCircle)
            p_dn_L1_IP.SetMarkerColor(ROOT.kRed)
        # draw
        canv3 = ROOT.TCanvas("L1_IP_prfx", "L1_IP_prfx", 1600, 800)
        canv3.Draw()
        p_up_L1_IP.Draw("psame")
        p_dn_L1_IP.Draw("psame")
        for line in lines2:
            line.Draw()
        for latex in latexs:
            latex.Draw()
        prof_L1_IP.Draw("psame")
        outdir2.cd()
#        febdir.cd()
        canv3.Write()
        canv3.Close()
                            

        
    if counter_L2_IP != 0:
        tmpx_L2_IP = hist_L2_IP.ProfileX("_tmpX_L2_IP", 1, hist_L2_IP.GetNbinsY(), "s")
        prof_L2_IP = ROOT.TH1D("L2_IP_prof", ";VMM*64 + Channel;Baseline RMS [%s]" % (ops.u), 8192, -0.5, 8191.5)
        for bin in range(0, tmpx_L2_IP.GetNbinsX()+1):
            multiplier = 1.0 if ops.u=="mV" else 6241.50975/9.0
            prof_L2_IP.SetBinContent(bin, tmpx_L2_IP.GetBinError(bin) * multiplier)
            prof_L2_IP.SetBinError  (bin, 0)
            if prof_L2_IP.GetBinContent(bin) > rms_of_interest:
                # bin 1 is channel 0
                print_me = "%s Bin %03i has RMS %4.1f => VMM/CH = %i/%02i"

        style(prof_L2_IP)
        prof_L2_IP.SetTitleSize(0.5)
#        prof_L2_IP.SetTitle("L2_IP")
        prof_L2_IP.SetMinimum(0)
        prof_L2_IP.SetMaximum(float(ops.y))
        prof_L2_IP.SetLineWidth(0)
        prof_L2_IP.SetMarkerStyle(ROOT.kFullCircle)
        prof_L2_IP.SetMarkerSize(0.2)
        prof_L2_IP.SetMarkerColor(ROOT.kBlack)

# VMM mean/median
        latexs = []
        for mmfe8_latex in range(0, 16):
            x = mmfe8_latex*512
            mean   = prof_L2_IP.Integral(x+1, x+512) / 512.0
            median = sorted([prof_L2_IP.GetBinContent(bin) for bin in xrange(x+1, x+512+1)]) [512/2]
            val    = median # change me at your desire
            string = ("%.2f" % (val)) if ops.u=="mV" else str(int(val))
            latexs.append( ROOT.TLatex(x+512/2, float(ops.y)*1.02, string) )
            latexs[-1].SetTextSize(0.025)
            latexs[-1].SetTextFont(42)
            latexs[-1].SetTextAlign(21)
            lines.append( ROOT.TLine(x, val, x+64, val) )
            lines[-1].SetLineColor(ROOT.kBlack)
            lines[-1].SetLineStyle(1)
            lines[-1].SetLineWidth(1)
            
            
            # clone for overflow
            p_up_L2_IP = prof_L2_IP.Clone(prof_L2_IP.GetName()+"_clone_up")
            for bin in range(0, p_up_L2_IP.GetNbinsX()+1):
                if p_up_L2_IP.GetBinContent(bin) > float(ops.y):
                    p_up_L2_IP.SetBinContent(bin, float(ops.y)-0.01)
                else:
                    p_up_L2_IP.SetBinContent(bin, -1)
            style(p_up_L2_IP)
            p_up_L2_IP.SetMinimum(0)
            p_up_L2_IP.SetMaximum(float(ops.y))
            p_up_L2_IP.SetMarkerStyle(ROOT.kFullTriangleUp)
            p_up_L2_IP.SetMarkerColor(ROOT.kRed)
            p_up_L2_IP.SetMarkerSize(1.0)
            
            # clone for underflow
            p_dn_L2_IP = prof_L2_IP.Clone(prof_L2_IP.GetName()+"_clone_dn")
            for bin in range(0, p_dn_L2_IP.GetNbinsX()+1):
                if p_dn_L2_IP.GetBinContent(bin) < 0.1:
                    p_dn_L2_IP.SetBinContent(bin, 0.01)
                else:
                    p_dn_L2_IP.SetBinContent(bin, -1)
            style(p_dn_L2_IP)
            p_dn_L2_IP.SetMinimum(0)
            p_dn_L2_IP.SetMaximum(float(ops.y))
            p_dn_L2_IP.SetMarkerStyle(ROOT.kFullCircle)
            p_dn_L2_IP.SetMarkerColor(ROOT.kRed)
        # draw
        canv3 = ROOT.TCanvas("L2_IP_prfx", "L2_IP_prfx", 1600, 800)
        canv3.Draw()
        p_up_L2_IP.Draw("psame")
        p_dn_L2_IP.Draw("psame")
        for line in lines2:
            line.Draw()
        for latex in latexs:
            latex.Draw()
        prof_L2_IP.Draw("psame")
        outdir2.cd()
#        febdir.cd()
        canv3.Write()
        canv3.Close()
                            

    if counter_L3_IP != 0:
        tmpx_L3_IP = hist_L3_IP.ProfileX("_tmpX_L3_IP", 1, hist_L3_IP.GetNbinsY(), "s")
        prof_L3_IP = ROOT.TH1D("L3_IP_prof", ";VMM*64 + Channel;Baseline RMS [%s]" % (ops.u), 8192, -0.5, 8191.5)
        for bin in range(0, tmpx_L3_IP.GetNbinsX()+1):
            multiplier = 1.0 if ops.u=="mV" else 6241.50975/9.0
            prof_L3_IP.SetBinContent(bin, tmpx_L3_IP.GetBinError(bin) * multiplier)
            prof_L3_IP.SetBinError  (bin, 0)
            if prof_L3_IP.GetBinContent(bin) > rms_of_interest:
                # bin 1 is channel 0
                print_me = "%s Bin %03i has RMS %4.1f => VMM/CH = %i/%02i"
    
        style(prof_L3_IP)
        prof_L3_IP.SetTitleSize(0.5)
#        prof_L3_IP.SetTitle("L3_IP")
        prof_L3_IP.SetMinimum(0)
        prof_L3_IP.SetMaximum(float(ops.y))
        prof_L3_IP.SetLineWidth(0)
        prof_L3_IP.SetMarkerStyle(ROOT.kFullCircle)
        prof_L3_IP.SetMarkerSize(0.2)
        prof_L3_IP.SetMarkerColor(ROOT.kBlack)
        
        # VMM mean/median
        latexs = []
        for vmm in range(0, 8):
            x = vmm*64
            mean   = prof_L3_IP.Integral(x+1, x+512) / 512.0
            median = sorted([prof_L3_IP.GetBinContent(bin) for bin in xrange(x+1, x+512+1)]) [512/2]
            val    = median # change me at your desire
            string = ("%.2f" % (val)) if ops.u=="mV" else str(int(val))
            latexs.append( ROOT.TLatex(x+512/2, float(ops.y)*1.02, string) )
            latexs[-1].SetTextSize(0.05)
            latexs[-1].SetTextFont(42)
            latexs[-1].SetTextAlign(21)
            lines.append( ROOT.TLine(x, val, x+64, val) )
            lines[-1].SetLineColor(ROOT.kBlack)
            lines[-1].SetLineStyle(1)
            lines[-1].SetLineWidth(1)
            
            
            # clone for overflow
            p_up_L3_IP = prof_L3_IP.Clone(prof_L3_IP.GetName()+"_clone_up")
            for bin in range(0, p_up_L3_IP.GetNbinsX()+1):
                if p_up_L3_IP.GetBinContent(bin) > float(ops.y):
                    p_up_L3_IP.SetBinContent(bin, float(ops.y)-0.01)
                else:
                    p_up_L3_IP.SetBinContent(bin, -1)
            style(p_up_L3_IP)
            p_up_L3_IP.SetMinimum(0)
            p_up_L3_IP.SetMaximum(float(ops.y))
            p_up_L3_IP.SetMarkerStyle(ROOT.kFullTriangleUp)
            p_up_L3_IP.SetMarkerColor(ROOT.kRed)
            p_up_L3_IP.SetMarkerSize(1.0)
            
            # clone for underflow
            p_dn_L3_IP = prof_L3_IP.Clone(prof_L3_IP.GetName()+"_clone_dn")
            for bin in range(0, p_dn_L3_IP.GetNbinsX()+1):
                if p_dn_L3_IP.GetBinContent(bin) < 0.1:
                    p_dn_L3_IP.SetBinContent(bin, 0.01)
                else:
                    p_dn_L3_IP.SetBinContent(bin, -1)
            style(p_dn_L3_IP)
            p_dn_L3_IP.SetMinimum(0)
            p_dn_L3_IP.SetMaximum(float(ops.y))
            p_dn_L3_IP.SetMarkerStyle(ROOT.kFullCircle)
            p_dn_L3_IP.SetMarkerColor(ROOT.kRed)
        # draw
        canv3 = ROOT.TCanvas("L3_IP_prfx", "L3_IP_prfx", 1600, 800)
        canv3.Draw()
        p_up_L3_IP.Draw("psame")
        p_dn_L3_IP.Draw("psame")
        prof_L3_IP.Draw("psame")
        outdir2.cd()
#        febdir.cd()
        canv3.Write()
        canv3.Close()


    if counter_L4_IP != 0:
        tmpx_L4_IP = hist_L4_IP.ProfileX("_tmpX_L4_IP", 1, hist_L4_IP.GetNbinsY(), "s")
        prof_L4_IP = ROOT.TH1D("L4_IP_prof", ";VMM*64 + Channel;Baseline RMS [%s]" % (ops.u), 8192, -0.5, 8191.5)
        for bin in range(0, tmpx_L4_IP.GetNbinsX()+1):
            multiplier = 1.0 if ops.u=="mV" else 6241.50975/9.0
            prof_L4_IP.SetBinContent(bin, tmpx_L4_IP.GetBinError(bin) * multiplier)
            prof_L4_IP.SetBinError  (bin, 0)
            if prof_L4_IP.GetBinContent(bin) > rms_of_interest:
                # bin 1 is channel 0
                print_me = "%s Bin %03i has RMS %4.1f => VMM/CH = %i/%02i"

        style(prof_L4_IP)
        prof_L4_IP.SetTitleSize(0.5)
#        prof_L4_IP.SetTitle("L4_IP")
        prof_L4_IP.SetMinimum(0)
        prof_L4_IP.SetMaximum(float(ops.y))
        prof_L4_IP.SetLineWidth(0)
        prof_L4_IP.SetMarkerStyle(ROOT.kFullCircle)
        prof_L4_IP.SetMarkerSize(0.2)
        prof_L4_IP.SetMarkerColor(ROOT.kBlack)

# VMM mean/median
        latexs = []
        for mmfe8_latex in range(0, 16):
            x = mmfe8_latex*512
            mean   = prof_L4_IP.Integral(x+1, x+512) / 512.0
            median = sorted([prof_L4_IP.GetBinContent(bin) for bin in xrange(x+1, x+512+1)]) [512/2]
            val    = median # change me at your desire
            string = ("%.2f" % (val)) if ops.u=="mV" else str(int(val))
            latexs.append( ROOT.TLatex(x+512/2, float(ops.y)*1.02, string) )
            latexs[-1].SetTextSize(0.025)
            latexs[-1].SetTextFont(42)
            latexs[-1].SetTextAlign(21)
            lines.append( ROOT.TLine(x, val, x+64, val) )
            lines[-1].SetLineColor(ROOT.kBlack)
            lines[-1].SetLineStyle(1)
            lines[-1].SetLineWidth(1)
            
            
            # clone for overflow
            p_up_L4_IP = prof_L4_IP.Clone(prof_L4_IP.GetName()+"_clone_up")
            for bin in range(0, p_up_L4_IP.GetNbinsX()+1):
                if p_up_L4_IP.GetBinContent(bin) > float(ops.y):
                    p_up_L4_IP.SetBinContent(bin, float(ops.y)-0.01)
                else:
                    p_up_L4_IP.SetBinContent(bin, -1)
            style(p_up_L4_IP)
            p_up_L4_IP.SetMinimum(0)
            p_up_L4_IP.SetMaximum(float(ops.y))
            p_up_L4_IP.SetMarkerStyle(ROOT.kFullTriangleUp)
            p_up_L4_IP.SetMarkerColor(ROOT.kRed)
            p_up_L4_IP.SetMarkerSize(1.0)
            
            # clone for underflow
            p_dn_L4_IP = prof_L4_IP.Clone(prof_L4_IP.GetName()+"_clone_dn")
            for bin in range(0, p_dn_L4_IP.GetNbinsX()+1):
                if p_dn_L4_IP.GetBinContent(bin) < 0.1:
                    p_dn_L4_IP.SetBinContent(bin, 0.01)
                else:
                    p_dn_L4_IP.SetBinContent(bin, -1)
            style(p_dn_L4_IP)
            p_dn_L4_IP.SetMinimum(0)
            p_dn_L4_IP.SetMaximum(float(ops.y))
            p_dn_L4_IP.SetMarkerStyle(ROOT.kFullCircle)
            p_dn_L4_IP.SetMarkerColor(ROOT.kRed)
        # draw
        canv3 = ROOT.TCanvas("L4_IP_prfx", "L4_IP_prfx", 1600, 800)
        canv3.Draw()
        p_up_L4_IP.Draw("psame")
        p_dn_L4_IP.Draw("psame")
        for line in lines2:
            line.Draw()
        for latex in latexs:
            latex.Draw()
        prof_L4_IP.Draw("psame")
        outdir2.cd()
#        febdir.cd()
        canv3.Write()
        canv3.Close()
    canv4 = ROOT.TCanvas("All_Layers_RMS", "All_layers_RMS", 1600, 800)
    canv4.Divide(4,2)
    canv4.Draw()
    counter_canv = 1
    if counter_L1_HO != 0:
        canv4.cd(counter_canv)
        p_up_L1_HO.Draw("psame")
        p_dn_L1_HO.Draw("psame")
        prof_L1_HO.Draw("psame")
        counter_canv = counter_canv + 1
    if counter_L2_HO != 0:
        canv4.cd(counter_canv)
        p_up_L2_HO.Draw("psame")
        p_dn_L2_HO.Draw("psame")
        prof_L2_HO.Draw("psame")
        counter_canv = counter_canv + 1
    if counter_L3_HO != 0:
        canv4.cd(counter_canv)
        p_up_L3_HO.Draw("psame")
        p_dn_L3_HO.Draw("psame")
        prof_L3_HO.Draw("psame")
        counter_canv = counter_canv + 1
    if counter_L4_HO != 0:
        canv4.cd(counter_canv)
        p_up_L4_HO.Draw("psame")
        p_dn_L4_HO.Draw("psame")
        prof_L4_HO.Draw("psame")
        counter_canv = counter_canv + 1
    if counter_L1_IP != 0:
        if counter_canv < 5: counter_canv = 5
        canv4.cd(counter_canv)
        p_up_L1_IP.Draw("psame")
        p_dn_L1_IP.Draw("psame")
        prof_L1_IP.Draw("psame")
        counter_canv = counter_canv + 1
    if counter_L2_IP != 0:
        if counter_canv < 5: counter_canv = 5
        canv4.cd(counter_canv)
        p_up_L2_IP.Draw("psame")
        p_dn_L2_IP.Draw("psame")
        prof_L2_IP.Draw("psame")
        counter_canv = counter_canv + 1
    if counter_L3_IP != 0:
        if counter_canv < 5: counter_canv = 5
        canv4.cd(counter_canv)
        p_up_L3_IP.Draw("psame")
        p_dn_L3_IP.Draw("psame")
        prof_L3_IP.Draw("psame")
        counter_canv = counter_canv + 1
    if counter_L4_IP != 0:
        if counter_canv < 5: counter_canv = 5
        canv4.cd(counter_canv)
        p_up_L4_IP.Draw("psame")
        p_dn_L4_IP.Draw("psame")
        prof_L4_IP.Draw("psame")
        counter_canv = counter_canv + 1


    outdir2.cd()
    canv4.Write()
    canv4.Close()


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
            number_miss_channels = miss_lm1_p1_e
        elif name[18] == "5":
            number_miss_channels = miss_lm1_p5_e
        elif name[18] == "6":
            number_miss_channels = miss_lm2_p6_e
        elif name[18] == "8":
            number_miss_channels = miss_lm2_p8_e
        else:
            number_miss_channels = 0

    else:
        if name[18] == "1":
            number_miss_channels = miss_lm1_p1_s
        elif name[18] == "5":
            number_miss_channels = miss_lm1_p5_s
        elif name[18] == "6":
            number_miss_channels = miss_lm2_p6_s
        elif name[18] == "8":
            number_miss_channels = miss_lm2_p8_s
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

def inverse_channels(name):
    inverse = 0
    if((name[16] == "1" or name[16] == "3") and (name[22] == "L") and (name[20] == "I") and (name[18] == "1")):
         inverse = 1
    elif((name[16] == "1" or name[16] == "3") and (name[22] == "L") and (name[20] == "I") and (name[18] == "6")):
        inverse = 1
    elif((name[16] == "2" or name[16] == "4") and (name[22] == "R") and (name[20] == "I") and (name[18] == "1")):
        inverse = 1
    elif((name[16] == "2" or name[16] == "4") and (name[22] == "R") and (name[20] == "I") and (name[18] == "6")):
        inverse = 1
    elif((name[16] == "1" or name[16] == "3") and (name[22] == "R") and (name[20] == "H") and (name[18] == "1")):
        inverse = 1
    elif((name[16] == "1" or name[16] == "3") and (name[22] == "R") and (name[20] == "H") and (name[18] == "6")):
        inverse = 1
    elif((name[16] == "2" or name[16] == "4") and (name[22] == "L") and (name[20] == "H") and (name[18] == "1")):
        inverse = 1
    elif((name[16] == "2" or name[16] == "4") and (name[22] == "L") and (name[20] == "H") and (name[18] == "6")):
        inverse = 1

    return (inverse)

def fatal(msg):
    import sys
    sys.exit("Fatal error: %s" % (msg))

if __name__ == "__main__":
    main()
