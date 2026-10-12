# Design

Approved on 2026-10-12 through the explicit instruction to implement the supplied iteration 030 plan. Scope is local implementation and verification; integration, commits, publication, tags and uploads are excluded.

Extend the existing code surface with a private inline Item action host. Use Component.createObject with initial codeText/requestCopy properties, preserving required-property creation. Each instance receives an immutable text snapshot and a mutable validity token; invalidate before destruction, including host teardown. Rebuild on component/text changes. Check the created root is an Item, diagnose failures, and retain code rendering. Bind action width to the content width and height to clamped implicitHeight; offset existing code by this height. Reuse this renderer for recursive containers.

Playground uses its existing Tools module for a small QGuiApplication clipboard helper, and a Qt Quick Controls Button in a host header with a short feedback timer. Public library dependencies stay unchanged. New installed fixture selects the 1.2 contract alongside existing 1.0/1.1 fixtures.

Initial properties follow [Qt Component.createObject documentation](https://doc.qt.io/qt-6.12/qml-qtqml-component.html#createObject-method).

Verification uncovered the existing Arch recipe test’s Git dependency missing from the pinned CI image. Add Git in a final Docker layer after SDK provisioning as a narrow verification fix, allowing cached SDK layers to be reused; this changes the image fingerprint and leaves the selected Qt SDKs unchanged. Callback guards also check current document/component/positive width so requests stop immediately before deferred item deletion.
