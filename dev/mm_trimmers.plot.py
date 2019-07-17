import argparse,sys, time
import ROOT
ROOT.gROOT.SetBatch()

def options():
    parser = argparse.ArgumentParser(usage=__doc__, formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("-o",             help="Output ROOT file", default="out.root")
    parser.add_argument("--bl",           help="txt file of baselines")
    parser.add_argument("--th_untrimmed", help="txt file of untrimmed thresholds")
    parser.add_argument("--th_trimmed",   help="txt file of trimmed thresholds")
    return parser.parse_args()

def fatal(msg):
    sys.exit("Fatal: %s" % msg)

def main():

    ops = options()
    if not ops.bl:           fatal("Please give a --bl file")
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
    tr_th_pre  = ROOT.TTree("DATA", "")
    tr_th_post = ROOT.TTree("DATA", "")
    tr_bl      .ReadFile(ops.bl,           "data/C:MMFE8:vmm_id/D:channel_id:tpdac:thdac:trim:ADCsample", " ")
    tr_th_pre  .ReadFile(ops.th_untrimmed, "data/C:MMFE8:vmm_id/D:channel_id:tpdac:thdac:trim:ADCsample", " ")
    tr_th_post .ReadFile(ops.th_trimmed,   "data/C:MMFE8:vmm_id/D:channel_id:tpdac:thdac:trim:ADCsample", " ")

    # plot
    for feb in febs:
        print("Plotting %s" % (feb))
        plot(feb, outfile, tr_bl, tr_th_pre, tr_th_post)


def plot(feb, outfile, tr_bl, tr_th_pre, tr_th_post):

    # hists
    title = ";VMM*64 + Channel;ADC Sample [mV];"
    h2_bl      = ROOT.TH2D("h2_bl",      title, 512, -0.5, 511.5, 1000,   0, 1000)
    h2_th_pre  = ROOT.TH2D("h2_th_pre",  title, 512, -0.5, 511.5, 1000,   0, 1000)
    h2_th_post = ROOT.TH2D("h2_th_post", title, 512, -0.5, 511.5, 1000,   0, 1000)

    # loop
    cmd = "ADCsample*1500.0/4095.0 : channel_id + vmm_id*64"
    req = "strstr(MMFE8,\"%s\")" % (feb)
    # req = "MMFE8 == %s" % (feb)
    tr_bl      .Draw(cmd + " >> h2_bl",      req, "colz")
    tr_th_pre  .Draw(cmd + " >> h2_th_pre",  req, "colz")
    tr_th_post .Draw(cmd + " >> h2_th_post", req, "colz")

    # profile
    h1_bl      = h2_bl     .ProfileX("h1_bl",      1, -1, "s")
    h1_th_pre  = h2_th_pre .ProfileX("h1_th_pre",  1, -1, "s")
    h1_th_post = h2_th_post.ProfileX("h1_th_post", 1, -1, "s")

    # diff
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
