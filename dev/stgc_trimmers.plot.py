import argparse,sys, time
import ROOT
ROOT.gROOT.SetBatch()

def options():
    parser = argparse.ArgumentParser(usage=__doc__, formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("-o",             help="Output ROOT file", default="out.root")
    parser.add_argument("--bl",           help="txt file of baselines")
    parser.add_argument("--bl_summ",      help="txt file of summary of baselines")
    parser.add_argument("--th_untrimmed", help="txt file of untrimmed thresholds")
    parser.add_argument("--th_trimmed",   help="txt file of trimmed thresholds")
    return parser.parse_args()

def fatal(msg):
    sys.exit("Fatal: %s" % msg)

def main():

    ops = options()
    if not ops.bl:           fatal("Please give a --bl file")
    if not ops.bl_summ:      fatal("Please give a --bl_summ file") # @ Prachi - patmasid
    if not ops.th_untrimmed: fatal("Please give a --th_untrimmed file")
    if not ops.th_trimmed:   fatal("Please give a --th_trimmed file")

    rootlogon()

    # output
    outfile = ROOT.TFile(ops.o, "recreate")

    # parse febs from input
    lines = open(ops.th_untrimmed).readlines()
    febs = sorted( list( set( [ line.split()[1] for line in lines ] ) ) )

    # get the data
    print("Loading data...")
    tr_bl      = ROOT.TTree("DATA", "")
    tr_bl_summ = ROOT.TTree("DATA", "")
    tr_th_pre  = ROOT.TTree("DATA", "")
    tr_th_post = ROOT.TTree("DATA", "")
    tr_bl      .ReadFile(ops.bl,           "data/C:MMFE8:vmm_id/D:channel_id:tpdac:thdac:trim:ADCsample", " ")
    tr_bl_summ .ReadFile(ops.bl_summ,      "data/C:sTGC_Board:vmm_str/C:vmm_id/D:channel_str/C:channel_id/D:mean_str/C:mean/D:stdev_str/C:stdev/D:median_str/C:median/D", " ")
    tr_th_pre  .ReadFile(ops.th_untrimmed, "data/C:MMFE8:vmm_id/D:channel_id:tpdac:thdac:trim:ADCsample", " ")
    tr_th_post .ReadFile(ops.th_trimmed,   "data/C:MMFE8:vmm_id/D:channel_id:tpdac:thdac:trim:ADCsample", " ")

    # plot
    for feb in febs:
        print("Plotting %s" % (feb))
        plot(feb, outfile, tr_bl, tr_bl_summ, tr_th_pre, tr_th_post)
        #plot_gaussian(feb, outfile, tr_bl, tr_bl_summ)

def plot(feb, outfile, tr_bl, tr_bl_summ, tr_th_pre, tr_th_post):

    ispFEB = None
    
    if(feb[:4]=="PFEB"):
        ispFEB = True
    else:
        ispFEB = False

    print ispFEB    

    # hists
    title = ";VMM*64 + Channel;ADC Sample [mV];"
    h2_bl      = ROOT.TH2D("h2_bl",      title, 512, -0.5, 511.5, 1000,   0, 1000)
    h2_th_pre  = ROOT.TH2D("h2_th_pre",  title, 512, -0.5, 511.5, 1000,   0, 1000)
    h2_th_post = ROOT.TH2D("h2_th_post", title, 512, -0.5, 511.5, 1000,   0, 1000)

    # loop
    cmd = "ADCsample*1000.0/4095.0 : channel_id + vmm_id*64"
    req = "strstr(MMFE8,\"%s\")" % (feb)
    # req = "MMFE8 == %s" % (feb)
    tr_bl      .Draw(cmd + " >> h2_bl",      req, "colz")
    tr_th_pre  .Draw(cmd + " >> h2_th_pre",  req, "colz")
    tr_th_post .Draw(cmd + " >> h2_th_post", req, "colz")

    # profile
    h1_bl      = h2_bl     .ProfileX("h1_bl",      1, -1, "s")
    h1_th_pre  = h2_th_pre .ProfileX("h1_th_pre",  1, -1, "s")
    h1_th_post = h2_th_post.ProfileX("h1_th_post", 1, -1, "s")

    #vmm_start << std::endl    # diff
    # dont try to diff TProfile :(
    nbins = h1_th_pre.GetNbinsX()
    xlo   = h1_th_pre.GetBinLowEdge(1)
    xhi   = h1_th_pre.GetBinLowEdge(nbins+1)
    h1_df_pre  = ROOT.TH1D("h1_df_pre",  "", nbins, xlo, xhi)
    h1_df_post = ROOT.TH1D("h1_df_post", "", nbins, xlo, xhi)
    for bin in range(h1_df_pre.GetNbinsX()+1):
        h1_df_pre .SetBinContent(bin, h1_th_pre .GetBinContent(bin) - h1_bl.GetBinContent(bin))
        h1_df_post.SetBinContent(bin, h1_th_post.GetBinContent(bin) - h1_bl.GetBinContent(bin))
        h1_df_pre .SetBinError  (bin, 0.0)
        h1_df_post.SetBinError  (bin, 0.0)


    hists = [h1_bl,
             h1_th_pre, h1_th_post,
             h1_df_pre, h1_df_post,
             ]

    # style
    for hist in hists:
        style(hist)
        hist.SetMarkerSize(0.5)
        #hist.SetMaximum(300)
        hist.SetMaximum(250)
        hist.SetMinimum(  0)
        #hist.GetXaxis().SetRangeUser(63.5, 127.5)
        hist.GetXaxis().SetTitle("VMM*64 + Channel")
        hist.GetYaxis().SetTitle("Mean ADC Sample [mV]")
    h1_bl     .SetMarkerColor(ROOT.kBlack)
    h1_th_pre .SetMarkerColor(ROOT.kBlue)
    h1_th_post.SetMarkerColor(ROOT.kBlue)
    h1_df_pre .SetMarkerColor(210)
    h1_df_post.SetMarkerColor(210)
    h1_bl     .SetLineColor(ROOT.kBlack)
    h1_th_pre .SetLineColor(ROOT.kBlue)
    h1_th_post.SetLineColor(ROOT.kBlue)
    h1_df_pre .SetLineColor(210)
    h1_df_post.SetLineColor(210)
    h1_bl     .SetMarkerStyle(ROOT.kFullCircle)
    h1_th_pre .SetMarkerStyle(ROOT.kOpenCircle)
    h1_df_pre .SetMarkerStyle(ROOT.kOpenCircle)
    h1_th_post.SetMarkerStyle(ROOT.kFullCircle)
    h1_df_post.SetMarkerStyle(ROOT.kFullCircle)

    # text
    boardname = ROOT.TLatex(0.22, 0.935, feb)
    # benchorno = ROOT.TLatex(0.62, 0.935, "On-chamber: PCB %s" % (int(feb)+1))
    texs = [boardname]
    for tex in texs:
        style(tex)

    # legend
    legend = ROOT.TLegend(0.39, 0.33, 0.51, 0.51)
    style(legend)
    legend.AddEntry(h1_th_pre,  "Threshold (+ arb. offset)",           "pe")
    legend.AddEntry(h1_th_post, "Threshold, with trimmers",            "pe")
    legend.AddEntry(h1_bl,      "Baseline",                            "pe")
    legend.AddEntry(h1_df_pre,  "Thr. #minus baseline",                "pe")
    legend.AddEntry(h1_df_post, "Thr. #minus baseline, with trimmers", "pe")

    # draw, zoom
    canv = ROOT.TCanvas("summary_zoom_%s" % (feb), 
                        "summary_zoom_%s" % (feb), 800, 800)
    canv.Draw()
    h1_bl     .Draw("pesame")
    h1_th_pre .Draw("pesame")
    h1_th_post.Draw("pesame")
    h1_df_pre .Draw("pesame")
    h1_df_post.Draw("pesame")
    legend.Draw()
    for tex in texs:
        tex.Draw()
    # canv.SaveAs("%s.pdf" % (canv.GetName()))
    outdir = outfile.Get("zoom")
    if not outdir:
        outdir = outfile.mkdir("zoom")
    outdir.cd()
    canv.Write()

    # draw, unzoom
    for hist in hists:
        hist.SetMaximum(1200)
    canv = ROOT.TCanvas("summary_unzoom_%s" % (feb), 
                        "summary_unzoom_%s" % (feb), 800, 800)
    canv.Draw()
    h1_bl     .Draw("pesame")
    h1_th_pre .Draw("pesame")
    h1_th_post.Draw("pesame")
    h1_df_pre .Draw("pesame")
    h1_df_post.Draw("pesame")
    legend.Draw()
    for tex in texs:
        tex.Draw()
    # canv.SaveAs("%s.pdf" % (canv.GetName()))
    outdir = outfile.Get("unzoom")
    if not outdir:
        outdir = outfile.mkdir("unzoom")
    outdir.cd()
    canv.Write()

    # draw, megazoom
    for hist in hists:
        hist.SetMaximum(70)
    canv = ROOT.TCanvas("summary_megazoom_%s" % (feb), 
                        "summary_megazoom_%s" % (feb), 800, 800)
    canv.Draw()
    h1_bl     .Draw("pesame")
    h1_th_pre .Draw("pesame")
    h1_th_post.Draw("pesame")
    h1_df_pre .Draw("pesame")
    h1_df_post.Draw("pesame")
    # legend.Draw()
    for tex in texs:
        tex.Draw()
    # canv.SaveAs("%s.pdf" % (canv.GetName()))
    outdir = outfile.Get("megazoom")
    if not outdir:
        outdir = outfile.mkdir("megazoom")
    outdir.cd()
    canv.Write()

    # legend for only baseline                                                                                                                                                                                                
    legend_only_bl = ROOT.TLegend(0.70, 0.70, 0.90, 0.90)
    style(legend_only_bl)
    legend_only_bl.AddEntry(h1_bl,      "Baseline",                            "pe")

    # draw only_baseline
    for hist in hists:
        hist.SetMaximum(400)
    canv = ROOT.TCanvas("only_baseline_%s" % (feb),
                        "only_baseline_%s" % (feb), 800, 800)
    canv.Draw()
    h1_bl     .Draw("pesame")
    legend_only_bl.Draw()                                                                                                                                                                                  

    for tex in texs:
        tex.Draw()
    # canv.SaveAs("%s.pdf" % (canv.GetName()))                                                                                                                                                              
    outdir = outfile.Get("only_baseline")
    if not outdir:
        outdir = outfile.mkdir("only_baseline")
    outdir.cd()
    canv.Write()

    ## Plots dependent on whether it is PFEB or SFEB

    h2_bl_stdev= ROOT.TH2D("h2_bl_stdev",title, 512, -0.5, 511.5, 100000,   0, 1000)
    
    cmd_2 = "stdev*1000.0/4095.0 : channel_id + vmm_id*64"
    req_2 = "strstr(sTGC_Board,\"%s\")" % (feb)
    tr_bl_summ .Draw(cmd_2 + " >> h2_bl_stdev",      req_2, "colz")
    
    h1_bl_stdev = h2_bl_stdev.ProfileX("h1_bl_stdev",      1, -1, "s")
    
    style(h1_bl_stdev)
    h1_bl_stdev.SetMarkerSize(1)
    #h1_bl_stdev.SetMaximum(250)
    h1_bl_stdev.SetMinimum(  0)
    h1_bl_stdev.GetXaxis().SetTitle("VMM*64 + Channel")
    h1_bl_stdev.GetYaxis().SetTitle("Stdev on noise ADC Sample [mV]")
    h1_bl_stdev     .SetMarkerColor(ROOT.kBlue)
    h1_bl_stdev     .SetLineColor(ROOT.kBlue)
    h1_bl_stdev     .SetMarkerStyle(ROOT.kFullSquare)
    
    # STDEV versus channel 

    if(ispFEB==False):
    
        h1_bl_stdev_strip = h1_bl_stdev.Clone("h1_bl_stdev_strip")
        #h1_bl_stdev_strip = h1_bl_stdev
    
        # legend for stdev of baseline versus channel 
        
        legend_only_stdev_strip = ROOT.TLegend(0.70, 0.70, 0.90, 0.90)
        style(legend_only_stdev_strip)
        legend_only_stdev_strip.AddEntry(h1_bl_stdev_strip,      "Strip- stdev on Baseline",                            "p")
        #if(feb[-2:]=="Q1"):
        #    legend_only_stdev_strip.AddEntry(h1_bl_stdev_strip, "CPi = 200 pF", "p")
        #elif(feb[-2:]=="Q2"):
        #    legend_only_stdev_strip.AddEntry(h1_bl_stdev_strip, "CPi = 330 pF", "p")
        #elif(feb[-2:]=="Q3"):
        #    legend_only_stdev_strip.AddEntry(h1_bl_stdev_strip, "CPi = 470 pF", "p")
        #h1_bl_stdev_strip.SetMaximum(5)
        canv = ROOT.TCanvas("strip_only_stdev_%s" % (feb),
                            "strip_only_stdev_%s" % (feb), 800, 800)
        canv.Draw()
        
        grid_strip = ROOT.TPad("grid_strip","",0,0,1,1);
        grid_strip.Draw("SAME");
        grid_strip.cd();
        grid_strip.SetFillStyle(4000);
        
        hgrid_strip = ROOT.TH2C("hgrid_strip","",512,-0.5,511.5,5,0.,h1_bl_stdev_strip.GetMaximum()*1.5);
        hgrid_strip.GetXaxis().SetTitle("VMM*64 + Channel")
        hgrid_strip.GetXaxis().SetTitleOffset(1)
        hgrid_strip.GetYaxis().SetTitle("Stdev on noise ADC Sample [mV]")
        hgrid_strip.GetYaxis().SetTitleOffset(1)
        hgrid_strip.Draw("SAME");
        h1_bl_stdev_strip     .Draw("pesame")
        hgrid_strip.GetXaxis().SetBit(ROOT.TAxis.kLabelsHori);
        hgrid_strip.GetXaxis().SetBinLabel(1,"0"); 
        for chan in range(1,17): 
            label_name = "%d" %(chan*32-1) 
            hgrid_strip.GetXaxis().SetBinLabel(chan*32,label_name)
        hgrid_strip.GetXaxis().SetNdivisions(-8);
        hgrid_strip.GetYaxis().SetNdivisions(-10);
        myline = ROOT.std.vector("TLine")(8)
        for chan2 in range(0,8):
            myline[chan2]=ROOT.TLine((chan2+1)*64-0.5,0,(chan2+1)*64-0.5,h1_bl_stdev_strip.GetMaximum()*1.5)
            myline[chan2].SetLineColor(ROOT.kRed)
            myline[chan2].SetLineWidth(1)
            myline[chan2].SetLineStyle(10)
            myline[chan2].Draw("same")
        legend_only_stdev_strip.Draw()
        
        for tex in texs:
            tex.Draw()
        outdir = outfile.Get("only_stdev")
        if not outdir:
            outdir = outfile.mkdir("only_stdev")
        outdir.cd()
        h1_bl_stdev_strip.Write()
        canv.Write()
        
    elif(ispFEB):
        h1_bl_stdev_wire = h1_bl_stdev.Clone("h1_bl_stdev_wire")

        # legend for stdev of baseline versus channel for wires
        
        legend_only_stdev_wire = ROOT.TLegend(0.70, 0.70, 0.90, 0.90)
        style(legend_only_stdev_wire)
        legend_only_stdev_wire.AddEntry(h1_bl_stdev_wire,      "Wire - stdev on Baseline",                            "p")
        if(feb[-2:]=="Q1"):
            legend_only_stdev_wire.AddEntry(h1_bl_stdev_wire, "CPi = 200 pF", "p")
        elif(feb[-2:]=="Q2"):
            legend_only_stdev_wire.AddEntry(h1_bl_stdev_wire, "CPi = 330 pF", "p")
        elif(feb[-2:]=="Q3"):
            legend_only_stdev_wire.AddEntry(h1_bl_stdev_wire, "CPi = 470 pF", "p")
        h1_bl_stdev_wire.GetXaxis().SetRangeUser(-0.5,63.5)
        canv = ROOT.TCanvas("wire_only_stdev_%s" % (feb),
                            "wire_only_stdev_%s" % (feb), 800, 800)
        canv.Draw()
        
        grid_wire = ROOT.TPad("grid_wire","",0,0,1,1);
        grid_wire.Draw("SAME");
        grid_wire.cd();
        grid_wire.SetFillStyle(4000);
        
        hgrid_wire = ROOT.TH2C("hgrid_wire","",64,-0.5,63.5,5,0.,h1_bl_stdev_wire.GetMaximum()*1.5);
        hgrid_wire.GetXaxis().SetTitle("VMM*64 + Channel")
        hgrid_wire.GetXaxis().SetTitleOffset(1)
        hgrid_wire.GetYaxis().SetTitle("Stdev on noise ADC Sample [mV]")
        hgrid_wire.GetYaxis().SetTitleOffset(1)
        hgrid_wire.Draw("SAME");
        h1_bl_stdev_wire.Draw("pesame")
        hgrid_wire.GetXaxis().SetBit(ROOT.TAxis.kLabelsHori);
        hgrid_wire.GetXaxis().SetBinLabel(1,"0");
        for chan in range(1,3):
            label_name = "%d" %(chan*32-1)
            hgrid_wire.GetXaxis().SetBinLabel(chan*32,label_name)
        hgrid_wire.GetXaxis().SetNdivisions(-8);
        hgrid_wire.GetYaxis().SetNdivisions(-10);
        myline = ROOT.std.vector("TLine")(2)
        for chan2 in range(0,2):
            myline[chan2]=ROOT.TLine((chan2+1)*64-0.5,0,(chan2+1)*64-0.5,h1_bl_stdev_wire.GetMaximum()*1.5)
            myline[chan2].SetLineColor(ROOT.kRed)
            myline[chan2].SetLineWidth(1)
            myline[chan2].SetLineStyle(10)
            myline[chan2].Draw("same")    
        legend_only_stdev_wire.Draw()

        for tex in texs:
            tex.Draw()
        outdir = outfile.Get("only_stdev")
        if not outdir:
            outdir = outfile.mkdir("only_stdev")
        outdir.cd()
        h1_bl_stdev_wire.Write()
        canv.Write()

        # legend for stdev of baseline versus channel for pads

        h1_bl_stdev_pad = h1_bl_stdev.Clone("h1_bl_stdev_pad")

        legend_only_stdev_pad = ROOT.TLegend(0.70, 0.70, 0.90, 0.90)
        style(legend_only_stdev_pad)
        legend_only_stdev_pad.AddEntry(h1_bl_stdev_pad,      "Pad - stdev on Baseline",                            "p")
        if(feb[-2:]=="Q1"):
            legend_only_stdev_pad.AddEntry(h1_bl_stdev_pad, "CPi = 200 pF", "p")
        elif(feb[-2:]=="Q2"):
            legend_only_stdev_pad.AddEntry(h1_bl_stdev_pad, "CPi = 330 pF", "p")
        elif(feb[-2:]=="Q3"):
            legend_only_stdev_pad.AddEntry(h1_bl_stdev_pad, "CPi = 470 pF", "p")
        h1_bl_stdev_pad.GetXaxis().SetRangeUser(63.5,191.5)
        canv = ROOT.TCanvas("pad_only_stdev_%s" % (feb),
                            "pad_only_stdev_%s" % (feb), 800, 800)
        canv.Draw()
        
        grid_pad = ROOT.TPad("grid_pad","",0,0,1,1);
        grid_pad.Draw("SAME");
        grid_pad.cd();
        grid_pad.SetFillStyle(4000);
        
        hgrid_pad = ROOT.TH2C("hgrid_pad","",128,63.5,191.5,5,0.,h1_bl_stdev_pad.GetMaximum()*1.5);
        hgrid_pad.GetXaxis().SetTitle("VMM*64 + Channel")
        hgrid_pad.GetXaxis().SetTitleOffset(1)
        hgrid_pad.GetYaxis().SetTitle("Stdev on noise ADC Sample [mV]")
        hgrid_pad.GetYaxis().SetTitleOffset(1)
        hgrid_pad.Draw("SAME");
        h1_bl_stdev_pad.Draw("pesame")
        hgrid_pad.GetXaxis().SetBit(ROOT.TAxis.kLabelsHori);
        hgrid_pad.GetXaxis().SetBinLabel(1,"64")
        for chan in range(3,7):
            label_name = "%d" %(chan*32-1)
            print label_name
            hgrid_pad.GetXaxis().SetBinLabel((chan-2)*32,label_name)
        hgrid_pad.GetXaxis().SetNdivisions(-8);
        hgrid_pad.GetYaxis().SetNdivisions(-10);
        myline = ROOT.std.vector("TLine")(4)
        for chan2 in range(0,4):
            myline[chan2]=ROOT.TLine((chan2+1)*64-0.5,0,(chan2+1)*64-0.5,h1_bl_stdev_pad.GetMaximum()*1.5)
            myline[chan2].SetLineColor(ROOT.kRed)
            myline[chan2].SetLineWidth(1)
            myline[chan2].SetLineStyle(10)
            myline[chan2].Draw("same")
        legend_only_stdev_pad.Draw()
        
        for tex in texs:
            tex.Draw()
        outdir = outfile.Get("only_stdev")
        if not outdir:
            outdir = outfile.mkdir("only_stdev")
        outdir.cd()
        h1_bl_stdev_pad.Write()
        canv.Write()

        #h2_bl_stdev_pad=ROOT.TH2D("h2_bl_stdev",title, 64, -0.5, 63.5, 100000,   0, 1000)
        #h2_bl_stdev_pad=ROOT.TH2D("h2_bl_stdev",title, 128, 63.5, 191.5, 100000,   0, 1000) 
        

def plot_gaussian(feb, outfile, tr_bl, tr_bl_summ):

    h1_baseline_Gauss = ROOT.std.vector("TH1D")(512)
    
    title = ";Baseline ADC Sample [mV];Count;"

    '''for channel in range(512):                                                                                                                                                                           
       hist_name = "h1_baseline_Gauss_%d" %(channel)                                                                                                                                                        
       h1_baseline_Gauss[channel] = ROOT.TH1D(hist_name,      title, 1000,   0, 1000)  

       style(h1_baseline_Gauss[channel])
       h1_baseline_Gauss[channel].SetMarkerSize(1)
       h1_baseline_Gauss[channel].SetMaximum(1000)
       h1_baseline_Gauss[channel].SetMinimum(  0)
       h1_baseline_Gauss[channel].GetXaxis().SetTitle("Baseline ADC Sample [mV]")
       h1_baseline_Gauss[channel].GetYaxis().SetTitle("Count")
       h1_baseline_Gauss[channel]     .SetMarkerColor(ROOT.kBlue)
       h1_baseline_Gauss[channel]     .SetLineColor(ROOT.kBlue)
       h1_baseline_Gauss[channel]     .SetMarkerStyle(ROOT.kFullCircle)

    nEntries = tr_bl.GetEntries()
    for i in range(0, nEntries): 
        tr_bl.GetEntry(i) 
        #print "Branch MMFE8 Value ",tr_bl.MMFE8
        
        h1_baseline_Gauss[tr_bl.vmm_id*64+tr_bl.channel_id].Fill(tr_bl.ADCsample*1000.0/4095.0)

        # text 
       boardname = ROOT.TLatex(0.22, 0.935, feb)
       texs = [boardname]
       for tex in texs:
           style(tex)


     for channel in range(512):    
           # legend for baseline gaussian
           
           legend_baseline_gaus = ROOT.TLegend(0.70, 0.70, 0.90, 0.90)
           style(legend_baseline_gaus)
           legend_baseline_gaus.AddEntry(h1_baseline_Gauss[channel],      "Baseline Gaussian Fit",                            "p")
       
           # draw only_baseline 
           h1_baseline_Gauss[channel].SetMaximum(5)
           canv = ROOT.TCanvas("baseline_Gaussian_%s" % (feb),
                               "baseline_Gaussian_%s" % (feb), 800, 800)
           canv.Draw()
           h1_baseline_Gauss[channel]     .Draw("pesame")
           legend_baseline_gaus.Draw()
           
       for tex in texs:
           tex.Draw()
       # canv.SaveAs("%s.pdf" % (canv.GetName()))                                                                                                                                                              
       outdir = outfile.Get("Baseline_GaussianFits")
       if not outdir:
           outdir = outfile.mkdir("Baseline_GaussianFits")
       FEB_dir = "Baseline_GaussianFits/"+tr_bl.MMFE8

       #print FEB_dir

       #outdir_FEB = outfile.Get(FEB_dir)
       #if not FEB_dir:
       #    outdir_FEB = outfile.mkdir(FEB_dir)
       outdir.cd()
       canv.Write()'''    

def rootlogon():
    ROOT.gStyle.SetOptStat(0)
    ROOT.gStyle.SetPadTopMargin(0.07)
    ROOT.gStyle.SetPadRightMargin(0.05)
    ROOT.gStyle.SetPadBottomMargin(0.12)
    ROOT.gStyle.SetPadLeftMargin(0.19)
    ROOT.gStyle.SetTitleOffset(1.2, 'x')
    ROOT.gStyle.SetTitleOffset(1.8, 'y')
    ROOT.gStyle.SetTextSize(0.05)
    ROOT.gStyle.SetLabelSize(0.05, 'xyz')
    ROOT.gStyle.SetTitleSize(0.05, 'xyz')
    ROOT.gStyle.SetTitleSize(0.05, 't')
    ROOT.gStyle.SetPadTickX(1)
    ROOT.gStyle.SetPadTickY(1)

def style(hist):
    if isinstance(hist, ROOT.TLatex):
        hist.SetTextSize(0.040)
        hist.SetTextFont(42)
        hist.SetNDC()
        return
    if isinstance(hist, ROOT.TLegend):
        hist.SetBorderSize(0)
        hist.SetFillColor(0)
        hist.SetFillStyle(0)
        hist.SetTextSize(0.035)
        return
    size = 0.045
    hist.GetXaxis().SetTitleSize(size)
    hist.GetXaxis().SetLabelSize(size)
    hist.GetYaxis().SetTitleSize(size)
    hist.GetYaxis().SetLabelSize(size)
    hist.GetXaxis().SetTitleOffset(1.2)
    hist.GetYaxis().SetTitleOffset(1.8)

if __name__ == "__main__":
    main()
