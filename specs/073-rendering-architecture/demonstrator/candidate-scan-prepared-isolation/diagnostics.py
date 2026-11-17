"""Separate instrumented scanner copies; never linked into timed executables."""
import shutil
import prepare_build as b


def prepare():
    b.p.require(not (b.WORK/'diagnostics-started.json').exists(),'diagnostic build already attempted')
    b.p.write(b.WORK/'diagnostics-started.json',dict(experimentLaunches=0))
    relative='modules/skparagraph/src'
    for label in ('C','I'):
        stage=b.WORK/(label+'-diagnostic')
        shutil.copytree(b.SOURCE/'baseline',stage,symlinks=True)
        b.impl.diagnostic(stage)
        source=stage/relative
        # Test-only injection enters the actual final-addition fallback after a
        # candidate has been selected; the original reset/exhaustive path follows.
        h=source/'ParagraphImpl.h'
        h.write_text(h.read_text().replace('int scanMode=2;', 'int scanMode=2; bool forceLateFallback=false; size_t lateFallbackHits=0;'))
        p=source/'TextWrapper.cpp';text=p.read_text()
        needle='if (!scan_bound::finite(required)) { reason=scan_bound::intermediate; break; }'
        assert text.count(needle)==1
        text=text.replace(needle,'''if (parent->forceLateFallback && fallbackEnd) {
                        ++parent->lateFallbackHits;
                        reason=scan_bound::intermediate; break;
                    }
                    '''+needle)
        p.write_text(text)
        if label=='I':
            before=p.read_text()
            p.write_text(b.split.split((b.SOURCE/'baseline'/relative/'TextWrapper.cpp').read_text(),before))
            header=source/'TextWrapper.h'
            header.write_text(header.read_text().replace('\nprivate:\n','''
private:
    __attribute__((noinline)) void breakPaddedTextIntoLines(
        ParagraphImpl* parent, SkScalar maxWidth, const AddLineToParagraph& addLine);
''',1))
        (b.WORK/(label+'-diagnostic.patch')).write_text(b.impl.diff((b.SOURCE/'retained' if label=='C' else b.WORK/'isolated')/relative,source))
        b.command(['ninja','-C',stage/'out/comparison','-j2','skparagraph'],label+'-diagnostic-library.log')
        driver=b.WORK/(label+'-diagnostic-driver')
        shutil.copytree(b.HERE.parent/'candidate-scan-implementation',driver,ignore=shutil.ignore_patterns('__pycache__'))
        test=driver/'tests.cpp';text=test.read_text()
        needle='        std::cout<<"focused cases="'
        assert needle in text
        text=text.replace(needle,'''        // Diagnostic injection tests the extracted scanner's late fallback,
        // including its endpoint reset, against exhaustive mode on the same input.
        m["padding"]={4,2}; json lateOracle;
        for(int mode:{0,2}) {
            auto p=prepareBoundaryParagraph(m,block,fonts,true);
            auto impl=static_cast<ParagraphImpl*>(p.paragraph.get());
            impl->scanMode=mode; impl->forceLateFallback=(mode==2);
            impl->layout(400); auto g=geometry(p);
            if(!mode)lateOracle=g;
            else { require(impl->lateFallbackHits>0,"late fallback not exercised");
                   require(g==lateOracle,"late fallback geometry mismatch"); }
        }
        std::cout<<"late fallback passed\\n";
'''+needle)
        test.write_text(text)
        # The copied trace uses relative includes into sibling historical drivers.
        for file in driver.glob('*.cpp'):
            file.write_text(file.read_text().replace('"../', '"'+str(b.HERE.parent)+'/'))
        cmake=driver/'CMakeLists.txt'
        cmake.write_text(cmake.read_text().replace('../boundary-experiment',str(b.HERE.parent/'boundary-experiment')).replace('../ownership-experiment',str(b.HERE.parent/'ownership-experiment')))
        # Numeric tests include the byte-identical numerical header from C.
        assert (driver/'ScanBound.h').read_bytes()==(source/'ScanBound.h').read_bytes()
        build=b.WORK/(label+'-diagnostic-build')
        b.command(['cmake','-S',driver,'-B',build,'-G','Ninja','-DCMAKE_BUILD_TYPE=Release','-DSKIA='+str(stage),'-DCOUNTERS=ON'],label+'-diagnostic-configure.log')
        b.command(['cmake','--build',build,'-j2','--target','scan-tests','numeric-tests','fast-math-tests','--verbose'],label+'-diagnostic-build.log')
    b.p.write(b.WORK/'diagnostic-audit.json',dict(separateFromTimed=True,verticalOnlyPadding=True,
        lateFallback='test-only injection after selected endpoint; actual extracted helper and original reset/exhaustive fallback',
        historicalSuite='unchanged focused matrix plus late fallback case; numerical algorithms byte-identical'))


if __name__=='__main__':
    try:prepare()
    except Exception as ex:
        b.p.write(b.WORK/'failure.json',dict(status='incomplete',stage='diagnostic-build',error=type(ex).__name__+': '+str(ex),executableLaunches=0))
        raise
