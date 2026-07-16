import UIKit
import MetalKit

class ViewController: UIViewController {

    var mtkView: MTKView!
    var xrView: MTKView!
    var bnView: BNView?

    /// Script paths passed as launch arguments (validation runner, etc.).
    /// A non-empty result puts the app in validation mode: the render surface
    /// is frame-driven so `TestUtils.updateSize()` can size it to the
    /// reference-image dimensions.
    static func testScriptArguments() -> [String] {
        return ProcessInfo.processInfo.arguments.dropFirst().filter { $0.hasSuffix(".js") }
    }

    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)
        guard
            let appDelegate = UIApplication.shared.delegate as? AppDelegate,
            let runtime = appDelegate.runtime
        else { return }

        setupViews()

        // Hand the runtime a reference to the XR overlay so NativeXr
        // can render its content into a separate transparent layer
        // when an XR session is active. The runtime keeps the
        // overlay's visibility in sync with the XR session state on
        // its own — the host doesn't need to toggle anything.
        runtime.setXrView(xrView)

        // Attach BNView to the main MTKView. Because mtkView's delegate
        // is still nil at this point, BNView auto-installs a managed
        // `BNViewDelegate` that drives the per-frame render callback.
        // Resize is driven separately from `viewDidLayoutSubviews` below.
        // First attach on this runtime triggers GPU device construction +
        // plugin initialization on the JS thread + queued-script flush.
        bnView = BNView(runtime: runtime, view: mtkView)

        // Simple gesture recognizer: forwards touches to BNView.
        let recognizer = UIBabylonGestureRecognizer(
            target: self,
            onTouchDown: { [weak self] (id, x, y) in self?.bnView?.pointerDown(id: Int(id), x: CGFloat(x), y: CGFloat(y)) },
            onTouchMove: { [weak self] (id, x, y) in self?.bnView?.pointerMove(id: Int(id), x: CGFloat(x), y: CGFloat(y)) },
            onTouchUp:   { [weak self] (id, x, y) in self?.bnView?.pointerUp  (id: Int(id), x: CGFloat(x), y: CGFloat(y)) }
        )
        mtkView.addGestureRecognizer(recognizer)
    }

    override func viewDidLayoutSubviews() {
        super.viewDidLayoutSubviews()

        // Babylon Native owns the drawable size (BNView sets
        // autoResizeDrawable = NO), so MTKView no longer reports size
        // changes via its delegate. Drive resize explicitly from layout,
        // passing logical points; BNView applies the device-pixel-ratio
        // internally.
        guard let mtkView = mtkView, let bnView = bnView else { return }
        let size = mtkView.bounds.size
        if size.width > 0 && size.height > 0 {
            bnView.resize(width: UInt(size.width), height: UInt(size.height))
        }
    }

    func setupViews() {
        let validationMode = !ViewController.testScriptArguments().isEmpty

        mtkView = MTKView()
        view.addSubview(mtkView)
        if validationMode {
            // Frame-driven sizing so TestUtils.updateSize() can resize the
            // render surface to the reference-image dimensions. Start at the
            // full view bounds (non-zero) so engine init can begin before the
            // first updateSize() call; updateSize() then shrinks it to the
            // authored test resolution.
            mtkView.translatesAutoresizingMaskIntoConstraints = true
            mtkView.autoresizingMask = []
            mtkView.frame = view.bounds
        } else {
            mtkView.translatesAutoresizingMaskIntoConstraints = false
            view.addConstraints(NSLayoutConstraint.constraints(withVisualFormat: "|[mtkView]|", options: [], metrics: nil, views: ["mtkView" : mtkView]))
            view.addConstraints(NSLayoutConstraint.constraints(withVisualFormat: "V:|[mtkView]|", options: [], metrics: nil, views: ["mtkView" : mtkView]))
        }

        xrView = MTKView()
        xrView.translatesAutoresizingMaskIntoConstraints = false
        xrView.isUserInteractionEnabled = false
        xrView.isHidden = true
        view.addSubview(xrView)
        view.addConstraints(NSLayoutConstraint.constraints(withVisualFormat: "|[xrView]|", options: [], metrics: nil, views: ["xrView" : xrView]))
        view.addConstraints(NSLayoutConstraint.constraints(withVisualFormat: "V:|[xrView]|", options: [], metrics: nil, views: ["xrView" : xrView]))
    }
}
