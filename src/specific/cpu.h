#ifndef CPU_H
#define CPU_H

typedef signed long SLONG;

class CPUID {
public:
    CPUID()
    {
        detect();
    }

    ULONG processor() const
    {
        return family;
    }

    ULONG isMMX() const
    {
        return MMX;
    }

    void get_vendor(char vendor[13]) const
    {
        for (int i = 13; --i >= 0;)
            vendor[i] = CPUID::vendor[i];
    }

    ULONG RDTSC_available() const
    {
        return rdtscON;
    }

    ULONG CPUID_features() const
    {
        return features;
    }

    ULONG CPUID_version() const
    {
        return version;
    }

private:
    SLONG cpuidmax;
    ULONG version;
    ULONG features;
    ULONG family;
    ULONG rdtscON;
    ULONG rdpmcON;
    ULONG MMX;
    char vendor[13];

    void detect()
    {
        SLONG cpuidmax = -1;
        ULONG version = 0;
        ULONG features = 0;
        ULONG processor = 0;
        ULONG rdtscON = 0;
        ULONG rdpmcON = 0;
        ULONG MMX = 0;
        char vendor[13] = "AnonymousCPU";

        __asm {
            pushad
            mov processor, 4
            pushfd
            pop eax
            mov ecx, eax
            xor eax, 200000h
            push eax
            popfd
            pushfd
            pop eax
            xor eax, ecx
            je endcpuid
            mov cpuidmax, 0
            mov eax, 0
            _emit 0x0f
            _emit 0xa2
            mov cpuidmax, eax
            mov dword ptr vendor, ebx
            mov dword ptr vendor[+4], edx
            mov dword ptr vendor[+8], ecx
            mov eax, 1
            _emit 0x0f
            _emit 0xa2
            mov version, eax
            mov features, edx
            shr eax, 8
            and eax, 0fh
            mov processor, eax
            cmp eax, 5
            jl endcpuid
            shr eax, 8
            and eax, 0100h
            setnz byte ptr rdpmcON
            and edx, 010h
            je endcpuid
            shr eax, 8
            and eax, 4
            setz byte ptr rdtscON
        endcpuid:
            popad
        }

        CPUID::cpuidmax = cpuidmax;
        CPUID::version = version;
        CPUID::features = features;
        CPUID::family = processor;
        CPUID::rdtscON = rdtscON;
        CPUID::rdpmcON = rdpmcON;
        CPUID::MMX = (CPUID::features >> 23) & 1;

        for (int i = 13; --i >= 0;)
            CPUID::vendor[i] = vendor[i];
    }
};

#endif
