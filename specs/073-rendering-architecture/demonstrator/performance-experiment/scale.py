#!/usr/bin/env python3
"""Extract scale facts; never reinterpret SDL's ratio as desktop scale."""
import argparse
import json
from pathlib import Path
import re


def diagnose(scale, protocol):
    def matches(pattern):
        return [list(map(int, m)) if isinstance(m, tuple) else int(m) for m in re.findall(pattern, protocol)]
    width, height = scale['windowCoordinates']
    dw, dh = scale['drawablePixels']
    assert width > 0 and height > 0
    assert abs(scale['drawablePerWindowX']-dw/width) < 1e-9
    assert abs(scale['drawablePerWindowY']-dh/height) < 1e-9
    assert scale['documentZoom'] == 1
    destinations = matches(r'wp_viewport#\d+\.set_destination\((\d+), (\d+)\)')
    fractional = matches(r'wp_fractional_scale_v1#\d+\.preferred_scale\((\d+)\)')
    buffers = matches(r'zwp_linux_buffer_params_v1#\d+\.create(?:_immed)?\((?:new id wl_buffer#\d+, )?(\d+), (\d+),')
    result = dict(sdl=scale, outputIntegerScale=matches(r'wl_output#\d+\.scale\((\d+)\)'),
        outputPixelModes=matches(r'wl_output#\d+\.mode\(\d+, (\d+), (\d+), (\d+)\)'),
        outputLogicalSize=matches(r'zxdg_output_v1#\d+\.logical_size\((\d+), (\d+)\)'),
        preferredFractionalScale120ths=fractional, viewportDestinations=destinations,
        surfaceBufferScaleRequests=matches(r'wl_surface#\d+\.set_buffer_scale\((\d+)\)'),
        windowGeometry=matches(r'xdg_surface#\d+\.set_window_geometry\((-?\d+), (-?\d+), (\d+), (\d+)\)'),
        viewportSourceRequests=re.findall(r'wp_viewport#\d+\.set_source\(([^)]+)\)', protocol),
        bufferDimensions=sorted(set(tuple(b) for b in buffers)), gaps=[])
    if destinations and fractional and buffers and [dw, dh] in buffers:
        sw, sh = destinations[-1]
        result['bufferPixelsPerSurfaceUnit'] = [dw/sw, dh/sh]
        result['sdlCoordinatesPerSurfaceUnit'] = [width/sw, height/sh]
        result['preferredDesktopScale'] = fractional[-1]/120
        result['mappingEstablished'] = abs(dw/sw-fractional[-1]/120) < 1e-9 and abs(dh/sh-fractional[-1]/120) < 1e-9
    else:
        result['mappingEstablished'] = False
        result['gaps'].append('Missing viewport destination, fractional event or matching buffer allocation.')
    if not result['windowGeometry']:
        result['gaps'].append('No explicit xdg set_window_geometry request; effective geometry is not independently reported.')
    result['outputNames'] = re.findall(r'wl_output#\d+\.name\("([^"]+)"\)', protocol)
    result['preferredSurfaceBufferScale'] = matches(r'wl_surface#\d+\.preferred_buffer_scale\((\d+)\)')
    result['effectiveSurfaceBufferScale'] = result['surfaceBufferScaleRequests'][-1] if result['surfaceBufferScaleRequests'] else 1
    result['surfaceBufferScaleBasis'] = 'last request' if result['surfaceBufferScaleRequests'] else 'Wayland initial value; no request in complete startup trace'
    result['viewportSurfaceLinks'] = matches(r'get_viewport\(new id wp_viewport#(\d+), wl_surface#(\d+)\)')
    result['resamplingKernel'] = None
    return result


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('directory', type=Path)
    args = p.parse_args()
    rows = {}
    for api in ('opengl', 'vulkan'):
        stem = args.directory/f'typical-padded-{api}-stationary-0'
        scale = json.loads(stem.with_suffix('.jsonl').read_text().splitlines()[0])['data']
        rows[api] = diagnose(scale, stem.with_suffix('.stderr').read_text())
    print(json.dumps(rows, indent=2))


if __name__ == '__main__':
    main()
