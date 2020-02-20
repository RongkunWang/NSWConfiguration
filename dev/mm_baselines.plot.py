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
    print "Data loaded."

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

        # draw
        canv = ROOT.TCanvas(name+"_prfx", name+"_prfx", 800, 800)
        canv.Draw()
        p_up.Draw("psame")
        p_dn.Draw("psame")
        for line in lines:
            line.Draw()
        for latex in latexs:
            latex.Draw()
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
    hist.GetXaxis().SetTitleSize(size)
    hist.GetXaxis().SetLabelSize(size)
    hist.GetYaxis().SetTitleSize(size)
    hist.GetYaxis().SetLabelSize(size)
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

def fatal(msg):
    import sys
    sys.exit("Fatal error: %s" % (msg))

if __name__ == "__main__":
    main()
