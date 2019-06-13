import argparse
import ROOT
ROOT.gROOT.SetBatch()

def options():
    parser = argparse.ArgumentParser(usage=__doc__, formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("-i", help="Input txt file of baseline measurements")
    parser.add_argument("-f", help="Comma-separated list of FEBs")
    parser.add_argument("-o", help="Output ROOT file of TCanvases", default="out.root")
    return parser.parse_args()

def main():

    ops = options()
    if not ops.i:
        fatal("Please give an input txt file with -i")
    if not ops.f:
        fatal("Please give a list of FEBs to measure baselines with -f")

    rootlogon()
    out = ROOT.TFile(ops.o, "recreate")

    # txt -> root
    tr = ROOT.TTree("DATA", "")
    tr.ReadFile("baselines.txt", "data/C:MMFE8:vmm_id/D:channel_id:tpdac:thdac:trim:ADCsample", " ")

    febs = ops.f.split(",")
    for feb in febs:
        if not feb:
            continue
        name = "baseline_MMFE8_%s" % (feb)
        hist = ROOT.TH2D(name, ";VMM*64 + Channel;Baseline [mV]; Samples", 512, -0.5, 511.5, 300, 0, 300)
        canv = ROOT.TCanvas(name+"_canv", name+"_canv", 800, 800)
        tr.Draw("ADCsample/4095 * 1.5 * 1000 : channel_id + 64*vmm_id >> %s" % (name), "strstr(MMFE8,\"%s\")" % (feb), "colz")
        style(hist)
        canv.Draw()
        hist.Draw("colzsame")
        canv.SaveAs(canv.GetName() + ".pdf")
        out.cd()
        canv.Write()

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
    hist.GetXaxis().SetTitleSize(size)
    hist.GetXaxis().SetLabelSize(size)
    hist.GetYaxis().SetTitleSize(size)
    hist.GetYaxis().SetLabelSize(size)
    hist.GetZaxis().SetTitleSize(size)
    hist.GetZaxis().SetLabelSize(size)
    hist.GetXaxis().SetTitleOffset(1.1)
    hist.GetYaxis().SetTitleOffset(1.8)
    hist.GetZaxis().SetTitleOffset(1.6)

def fatal(msg):
    import sys
    sys.exit("Fatal error: %s" % (msg))

if __name__ == "__main__":
    main()
