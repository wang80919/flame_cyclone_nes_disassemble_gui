#include "C6502Introduction.h"
#include <strsafe.h>
#include <Windows.h>
#include "Win32Utils/CStrUtils.h"

const int FORMAT_COUNT_MAX = (1024 * 1024 * 64);
_tstring Format(LPCTSTR pFormat, ...)
{
    size_t nCchCount = MAX_PATH;
    _tstring strResult(nCchCount, 0);
    va_list args;

    va_start(args, pFormat);

    do
    {
        //格式化输出字符串
        int nSize = _vsntprintf_s(&strResult[0], nCchCount, _TRUNCATE, pFormat, args);
        if (-1 != nSize)
        {
            strResult.resize(nSize);
            break;
        }

        //缓冲大小超限终止
        if (nCchCount >= FORMAT_COUNT_MAX)
        {
            break;
        }

        //重新分配缓冲
        nCchCount *= 2;
        strResult.resize(nCchCount);

    } while (true);

    va_end(args);

    return strResult;
}

//
// @brief: 获取指令列表
// @param: Introductions      指令列表
// @param: DataReserved         保留的数据
// @param: pBaseAddr            反汇编数据地址
// @param: size                 数据大小
// @param: addr                 反汇编地址
// @param: depth                递归深度
// @ret: void
static void _DisassembleInstructions(
    std::map<uint16_t, C6502Introduction>& introductionList,
    std::map<uint16_t, DATA_INFO>& dataList,
    uint8_t* pBaseAddr,
    uint32_t size,
    uint16_t addr,
    int32_t depth = 8
);

_tstring GetDisassembleText(
    std::map<uint16_t, C6502Introduction>& introductionList,
    const _tstring& strLabel,
    uint8_t* pBaseAddr,
    uint32_t size,
    bool bShowAddr/* = false*/,
    bool b6502SimulatorSyntax/* = false*/
)
{
    LPBYTE lpData = pBaseAddr;
    _tstring strOutput;
    for (auto& item : introductionList)
    {
        if (item.second.IsLabbeled())
        {
            if (strLabel.empty())
            {
                strOutput += CStrUtils::Format(_T("Label_%04X\r\n"), item.second.GetAddr());
            }
            else
            {
                strOutput += CStrUtils::Format(_T("%s%04X\r\n"), strLabel.c_str(), item.second.GetAddr());
            }
        }

        if (item.second.IsReserved())
        {
            if (1 == item.second.GetElemSize())
            {
                uint16_t addr = item.second.GetAddr();
                int16_t dataSize = item.second.GetData();
                if (dataSize > 0)
                {
                    _tstring strDB = _T("    .DB ");
                    for (int i = 0; i < dataSize; i++)
                    {
                        if (0 != i && (0 == i % 16))
                        {
                            strDB += _T("\r\n    .DB ");
                        }

                        if (0 == i % 16)
                        {
                            strDB += CStrUtils::Format(_T("$%02X"), lpData[addr + i]);
                        }
                        else
                        {
                            strDB += CStrUtils::Format(_T(",$%02X"), lpData[addr + i]);
                        }
                    }

                    strOutput += strDB;
                    strOutput += _T("\r\n");
                }
            }

            if (2 == item.second.GetElemSize())
            {
                uint16_t addr = item.second.GetAddr();
                int16_t dataSize = item.second.GetData();

                if (dataSize > 0)
                {
                    _tstring strDB = _T("    .DW ");
                    for (int i = 0; i < dataSize; i+= 2)
                    {
                        if (0 != i && (0 == i % 2))
                        {
                            strDB += _T("\r\n    .DW ");
                        }

                        uint16_t data = lpData[addr + i] | (lpData[addr + i + 1] << 8);

                        auto itFind = introductionList.find(data);
                        if (itFind != introductionList.end())
                        {
                            if (itFind->second.IsLabbeled())
                            {
                                if (strLabel.empty())
                                {
                                    if (0 == i % 2)
                                    {
                                        strDB += CStrUtils::Format(_T("Label_%04X"), data);
                                    }
                                    else
                                    {
                                        strDB += CStrUtils::Format(_T(",Label_%04X"), data);
                                    }
                                }
                                else
                                {
                                    if (0 == i % 2)
                                    {
                                        strDB += CStrUtils::Format(_T("%s%04X"), strLabel.c_str(), itFind->second.GetAddr());
                                    }
                                    else
                                    {
                                        strDB += CStrUtils::Format(_T(",%s%04X"), strLabel.c_str(), itFind->second.GetAddr());
                                    }
                                }
                            }
                            else
                            {
                                if (0 == i % 2)
                                {
                                    strDB += CStrUtils::Format(_T("$%04X"), data);
                                }
                                else
                                {
                                    strDB += CStrUtils::Format(_T(",$%04X"), data);
                                }
                            }
                        }
                        else
                        {
                            if (0 == i % 2)
                            {
                                strDB += CStrUtils::Format(_T("$%04X"), data);
                            }
                            else
                            {
                                strDB += CStrUtils::Format(_T(",$%04X"), data);
                            }
                        }
                    }

                    strOutput += strDB;
                    strOutput += _T("\r\n");
                }
            }
        }

        strOutput += CStrUtils::Format(_T("    %s\r\n"), item.second.GetFormat(strLabel, b6502SimulatorSyntax, bShowAddr).c_str());
        if (item.second.GetName() == _T("JMP") || item.second.GetName() == _T("RTS"))
        {
            strOutput += _T("\r\n");
        }
    }

    return strOutput;
}

void DisassembleInstructions(
    std::map<uint16_t, C6502Introduction>& IntroductionSet,
    std::map<uint16_t, DATA_INFO>& DataReserved,
    std::vector<uint16_t> vAddr,
    uint8_t* pBaseAddr,
    uint32_t size
)
{
    //设置保留数据段
    for (const auto& item : DataReserved)
    {
        C6502Introduction info(00, _T("Reserved"), e6502AddressingMode::eModeReserved, item.first, item.second.m_length, item.second.m_elem, true, true);
        IntroductionSet.insert(std::make_pair(item.first, info));
    }

    //反汇编每个指定地址
    for (const auto& item : vAddr)
    {
        _DisassembleInstructions(IntroductionSet, DataReserved, pBaseAddr, size, item, 16);
    }

    for (auto& item : IntroductionSet)
    {
        if (item.second.IsJmpAbsolute() || (_T("RTS") == item.second.GetName()))
        {
            auto itNext = IntroductionSet.find(item.first + item.second.GetSize());
            if (itNext != IntroductionSet.end())
            {
                itNext->second.SetLabbeled(true);
            }
        }
    }

    //标签排序
    std::map<uint16_t, uint16_t> labelCount;
    uint16_t nLabelIndex = 0;
    for (auto& item : IntroductionSet)
    {
        for (auto& addr : vAddr)
        {
            if (addr == item.first)
            {
                item.second.SetLabbeled(true);
            }
        }

        if (item.second.IsLabbeled())
        {
            item.second.SetLabelID(nLabelIndex);
            nLabelIndex++;
        }
    }

    for (auto& item : IntroductionSet)
    {
        if (item.second.IsJsrOrJmpAbsolute() || item.second.IsRelative())
        {
            uint16_t callAddr = item.second.GetCallAddr();

            if (item.second.GetLabelAddr())
            {
                callAddr = item.second.GetLabelAddr();
            }

            auto itFind = IntroductionSet.find(callAddr);
            if (IntroductionSet.end() != itFind)
            {
                item.second.SetRefID(itFind->second.GetLabelID());
            }
        }
    }

    for (auto& item : IntroductionSet)
    {
        if (item.second.IsLabbeled())
        {
            if (item.second.IsAbsoluteRead())
            {
                auto data = IntroductionSet.find(item.second.GetData());
                if (data != IntroductionSet.end())
                {
                    item.second.SetRefID(data->second.GetLabelID());
                }
            }
        }
    }

    return;
}

void _DisassembleInstructions(
    std::map<uint16_t, C6502Introduction>& introductionList,
    std::map<uint16_t, DATA_INFO>& dataList,
    uint8_t* pBaseAddr,
    uint32_t size,
    uint16_t addr,
    int32_t depth/* = 8*/
)
{
    //检测递归深度
    if (depth < 0)
    {
        return;
    }

    //反汇编地址检查
    uint8_t* pBegin = pBaseAddr + addr;
    if (pBegin >= pBaseAddr + size)
    {
        return;
    }

    uint16_t tmpAddr = addr;

    do
    {
        //查找指令, 找不到则结束
        auto itIntroduction = g_6502_Introduction_Set.find(*pBegin);
        if (itIntroduction == g_6502_Introduction_Set.end())
        {
            break;
        }

        bool bBreak = false;
        //遇到数据段则结束
        for (const auto& item : dataList)
        {
            if ((tmpAddr >= item.first) && (tmpAddr < (item.first + item.second.m_length)))
            {
                pBegin += item.second.m_length;
                tmpAddr += item.second.m_length;
                bBreak = true;
                break;
            }
        }

        if (bBreak)
        {
            continue;
        }

        //
        C6502Introduction Introduction(itIntroduction->second);
        uint8_t* pData = pBegin + 1;

        //获取指令数据
        if (1 == Introduction.GetDataSize())
        {
            uint8_t data = *(pData);
            Introduction.SetData(data);
        }
        else if (2 == Introduction.GetDataSize())
        {
            uint16_t data = *(uint16_t*)pData;
            for (auto& item : introductionList)
            {
                if (data > item.first && data <= (item.first + item.second.GetDataSize()))
                {
                    Introduction.SetLabelOffset(item.first, data - item.first);
                }
            }

            Introduction.SetData(data);
        }

        //首个指令设置一个标签记号
        if (addr == tmpAddr)
        {
            Introduction.SetLabbeled(true);
        }

        //设置指令地址
        Introduction.SetAddr(tmpAddr);

        if (Introduction.IsAbsoluteRead())
        {
            if (dataList.end() != dataList.find(Introduction.GetData()))
            {
                Introduction.SetLabbeled(true);
            }
        }

        //记录指令
        introductionList.insert(std::make_pair((uint16_t)tmpAddr, Introduction));

        //标号设置tmpAddr
        //遇到 相对跳转 或 JSR 或 JMP调用 子程序, 则递归处理
        if (Introduction.IsRelative() || Introduction.IsJsrOrJmpAbsolute())
        {
            uint16_t callAddr = Introduction.GetCallAddr();
            auto itFind = introductionList.find(callAddr);
            if (introductionList.end() != itFind)
            {
                itFind->second.SetLabbeled(true);
            }
            else
            {
                if (!Introduction.GetLabelAddr())
                {
                    _DisassembleInstructions(introductionList, dataList, pBaseAddr, size, callAddr, depth - 1);
                }
            }
        }

        //遇到子程序返回则结束
        if (_T("RTS") == Introduction.GetName())
        {
            // break;
        }

        //遇到中断返回则结束
        if (_T("RTI") == Introduction.GetName())
        {
            // break;
        }

        //遇到JMP跳转则结束
        if (_T("JMP") == Introduction.GetName())
        {
            // break;
        }

        //遇到子程序返回则结束
        if (_T("INT") == Introduction.GetName())
        {
            //break;
        }

        //解析指针向后增加
        pBegin += itIntroduction->second.GetSize();

        //指令地址向后增加
        tmpAddr += itIntroduction->second.GetSize();

    } while (pBegin < pBaseAddr + size);
}

_tstring C6502Introduction::GetName() const
{
    return m_name;
}

e6502AddressingMode C6502Introduction::GetAddressingMode() const
{
    return m_type;
}

_tstring C6502Introduction::GetFormat(const _tstring& strLabel/* = _T("")*/, bool b6502SimulatorSyntax/* = false*/, bool bShowAddr/* = false*/) const
{
    _tstring strResult;

    do
    {
        if (!strLabel.empty())
        {
            if ((IsRelative() || IsJsrOrJmpAbsolute()) && (uint16_t)-1 != m_refId)
            {
                uint16_t labOffset = GetLabelOffset();
                strResult = Format(_T("%s %s%04X"), m_name.c_str(), strLabel.c_str(), GetCallAddr());

                if (labOffset)
                {
                    strResult += Format(_T(" + %hd"), labOffset);
                }

                if (bShowAddr)
                {
                    strResult += Format(_T(";$%04X"), GetCallAddr());
                }
                break;
            }
        }
        else
        {
            if (IsRelative() && (uint16_t)-1 != m_refId)
            {
                strResult = Format(_T("%s Label_%04X"), m_name.c_str(), GetCallAddr());
                break;
            }

            if (IsJsrOrJmpAbsolute() && (uint16_t)-1 != m_refId)
            {
                uint16_t callAddr = GetCallAddr();
                uint16_t labOffset = GetLabelOffset();
                if (labOffset)
                {
                    callAddr = GetLabelAddr();
                }

                if (labOffset)
                {
                    strResult = Format(_T("%s Label_%04X + %hd"), m_name.c_str(), callAddr, labOffset);
                }
                else
                {
                    strResult = Format(_T("%s Label_%04X"), m_name.c_str(), callAddr);
                }
                break;
            }
        }

        if (e6502AddressingMode::eModeZeroPage == m_type)
        {
            if (b6502SimulatorSyntax)
            {
                strResult = Format(_T("%s $%02X"), m_name.c_str(), (uint8_t)m_data);
            }
            else
            {
                strResult = Format(_T("%s <$%02X"), m_name.c_str(), (uint8_t)m_data);
            }
            break;
        }
        
        if (e6502AddressingMode::eModeZeroPageX == m_type)
        {
            if (b6502SimulatorSyntax)
            {
                strResult = Format(_T("%s $%02X,X"), m_name.c_str(), (uint8_t)m_data);
            }
            else
            {
                strResult = Format(_T("%s <$%02X,X"), m_name.c_str(), (uint8_t)m_data);
            }
            break;
        }
        
        if (e6502AddressingMode::eModeZeroPageY == m_type)
        {
            if (b6502SimulatorSyntax)
            {
                strResult = Format(_T("%s $%02X,Y"), m_name.c_str(), (uint8_t)m_data);
            }
            else
            {
                strResult = Format(_T("%s <$%02X,Y"), m_name.c_str(), (uint8_t)m_data);
            }
            break;
        }
        
        if (e6502AddressingMode::eModeAbsolute == m_type)
        {
            if ((uint16_t)-1 != m_refId)
            {
                if (strLabel.empty())
                {
                    strResult = Format(_T("%s Label_%04X"), m_name.c_str(), m_data);
                }
                else
                {
                    if (bShowAddr)
                    {
                        strResult = Format(_T("%s %s%d;$%04X"), m_name.c_str(), strLabel.c_str(), m_refId, m_data);
                    }
                    else
                    {
                        strResult = Format(_T("%s %s%d,X"), m_name.c_str(), strLabel.c_str(), m_refId);
                    }
                }
            }
            else
            {
                strResult = Format(_T("%s $%04X"), m_name.c_str(), m_data);
            }
            break;
        }
        
        if (e6502AddressingMode::eModeAbsoluteX == m_type)
        {
            if ((uint16_t)-1 != m_refId)
            {
                if (strLabel.empty())
                {
                    strResult = Format(_T("%s Label_%04X,X"), m_name.c_str(), m_data);
                }
                else
                {
                    if (bShowAddr)
                    {
                        strResult = Format(_T("%s %s%d,X;$%04X"), m_name.c_str(), strLabel.c_str(), m_refId, m_data);
                    }
                    else
                    {
                        strResult = Format(_T("%s %s%d,X"), m_name.c_str(), strLabel.c_str(), m_refId);
                    }
                }
            }
            else
            {
                strResult = Format(_T("%s $%04X,X"), m_name.c_str(), m_data);
            }
            break;
        }
        
        if (e6502AddressingMode::eModeAbsoluteY == m_type)
        {
            if ((uint16_t)-1 != m_refId)
            {
                if (strLabel.empty())
                {
                    strResult = Format(_T("%s Label_%04X,Y"), m_name.c_str(), m_data);
                }
                else
                {
                    if (bShowAddr)
                    {
                        strResult = Format(_T("%s %s%d,Y;$%04X"), m_name.c_str(), strLabel.c_str(), m_refId, m_data);
                    }
                    else
                    {
                        strResult = Format(_T("%s %s%d,Y"), m_name.c_str(), strLabel.c_str(), m_refId);
                    }
                }
            }
            else
            {
                strResult = Format(_T("%s $%04X,Y"), m_name.c_str(), m_data);
            }

            break;
        }
        
        if (e6502AddressingMode::eModeAccumulator == m_type)
        {
            if (b6502SimulatorSyntax)
            {
                strResult = Format(_T("%s"), m_name.c_str());
            }
            else
            {
                strResult = Format(_T("%s A"), m_name.c_str());
            }
            break;
        }
        
        if (e6502AddressingMode::eModeImplicit == m_type)
        {
            strResult = Format(_T("%s"), m_name.c_str());
            break;
        }
        
        if (e6502AddressingMode::eModeIndirect == m_type)
        {
            if (b6502SimulatorSyntax)
            {
                strResult = Format(_T("%s ($%04X)"), m_name.c_str(), m_data);
            }
            else
            {
                strResult = Format(_T("%s [$%04X]"), m_name.c_str(), m_data);
            }
            break;
        }
        
        if (e6502AddressingMode::eModeIndexedIndirect == m_type)
        {
            if (b6502SimulatorSyntax)
            {
                strResult = Format(_T("%s ($%02X,X)"), m_name.c_str(), (uint8_t)m_data);
            }
            else
            {
                strResult = Format(_T("%s [$%02X,X]"), m_name.c_str(), (uint8_t)m_data);
            }
            break;
        }
        
        if (e6502AddressingMode::eModeIndirectIndexed == m_type)
        {
            if (b6502SimulatorSyntax)
            {
                strResult = Format(_T("%s ($%02X),Y"), m_name.c_str(), (uint8_t)m_data);
            }
            else
            {
                strResult = Format(_T("%s [$%02X],Y"), m_name.c_str(), (uint8_t)m_data);
            }
            break;
        }
        
        if (e6502AddressingMode::eModeImmediate == m_type)
        {
            strResult = Format(_T("%s #$%02X"), m_name.c_str(), (uint8_t)m_data);
            break;
        }

        if (e6502AddressingMode::eModeRelative == m_type)
        {
            strResult = Format(_T("%s $%04X"), m_name.c_str(), GetCallAddr());
            break;
        }

    } while (false);

    return strResult;
}

bool C6502Introduction::IsJmpAbsolute() const
{
    if ((_T("JMP") == m_name && e6502AddressingMode::eModeAbsolute == m_type))
    {
        return true;
    }

    return false;
}

bool C6502Introduction::IsJsrAbsolute() const
{
    if ((_T("JSR") == m_name && e6502AddressingMode::eModeAbsolute == m_type))
    {
        return true;
    }

    return false;
}

bool C6502Introduction::IsAbsoluteRead() const
{
    if (IsJsrOrJmpAbsolute())
    {
        return false;
    }

    if (_T("STA") == GetName() ||
        _T("ASL") == GetName() ||
        _T("LSR") == GetName() ||
        _T("ROL") == GetName() ||
        _T("ROR") == GetName() ||
        _T("INC") == GetName() ||
        _T("DEC") == GetName()
        )
    {
        return false;
    }

    if (e6502AddressingMode::eModeAbsolute == m_type ||
        e6502AddressingMode::eModeAbsoluteX == m_type ||
        e6502AddressingMode::eModeAbsoluteY == m_type)
    {
        return true;
    }

    return false;
}

bool C6502Introduction::IsJsrOrJmpAbsolute() const
{
    return IsJsrAbsolute() || IsJmpAbsolute();
}

bool C6502Introduction::IsRelative() const
{
    if (e6502AddressingMode::eModeRelative == m_type)
    {
        return true;
    }

    return false;
}

uint16_t C6502Introduction::GetCallAddr() const
{
    if (IsJsrOrJmpAbsolute())
    {
        return m_data;
    }

    if (IsRelative())
    {
        return GetNextAddr() + (int8_t)m_data;
    }

    return 0;
}

bool C6502Introduction::IsReserved() const
{
    return m_bReserved;
}

void C6502Introduction::SetReserved(bool bData)
{
    m_bReserved = bData;
}

bool C6502Introduction::IsLabbeled() const
{
    return m_bHasId;
}

void C6502Introduction::SetLabbeled(bool bLabled)
{
    m_bHasId = bLabled;
}

uint8_t C6502Introduction::GetDataSize() const
{
    if (e6502AddressingMode::eModeZeroPage == m_type)
    {
        return 1;
    }
    else if (e6502AddressingMode::eModeZeroPageX == m_type)
    {
        return 1;
    }
    else if (e6502AddressingMode::eModeZeroPageY == m_type)
    {
        return 1;
    }
    else if (e6502AddressingMode::eModeAbsolute == m_type)
    {
        return 2;
    }
    else if (e6502AddressingMode::eModeAbsoluteX == m_type)
    {
        return 2;
    }
    else if (e6502AddressingMode::eModeAbsoluteY == m_type)
    {
        return 2;
    }
    else if (e6502AddressingMode::eModeAccumulator == m_type)
    {
        return 0;
    }
    else if (e6502AddressingMode::eModeImplicit == m_type)
    {
        return 0;
    }
    else if (e6502AddressingMode::eModeIndirect == m_type)
    {
        return 2;
    }
    else if (e6502AddressingMode::eModeIndexedIndirect == m_type)
    {
        return 1;
    }
    else if (e6502AddressingMode::eModeIndirectIndexed == m_type)
    {
        return 1;
    }
    else if (e6502AddressingMode::eModeImmediate == m_type)
    {
        return 1;
    }
    else if (e6502AddressingMode::eModeRelative == m_type)
    {
        return 1;
    }

    return 0;
}

uint8_t C6502Introduction::GetSize() const
{
    return GetDataSize() + 1;
}

uint16_t C6502Introduction::GetAddr() const
{
    return m_addr;
}

uint16_t C6502Introduction::GetNextAddr() const
{
    return m_addr + GetDataSize() + 1;
}

uint16_t C6502Introduction::GetData() const
{
    return m_data;
}

uint16_t C6502Introduction::GetElemSize() const
{
    return m_elem;
}

void C6502Introduction::SetElemSize(uint8_t elem)
{
    m_elem = elem;
}

uint16_t C6502Introduction::GetLabelID() const
{
    return m_id;
}

void C6502Introduction::SetLabelID(uint16_t id)
{
    m_id = id;
}

uint16_t C6502Introduction::GetRefID() const
{
    return m_refId;
}

void C6502Introduction::SetRefID(uint16_t id)
{
    m_refId = id;
}

void C6502Introduction::SetAddr(uint16_t addr)
{
    m_addr = addr;
}

void C6502Introduction::SetData(uint16_t data)
{
    m_data = data;
}

void C6502Introduction::SetLabelOffset(uint16_t data, uint8_t offset)
{
    m_labStart = data;
    m_labOffset = offset;
}

uint16_t C6502Introduction::GetLabelAddr() const
{
    return m_labStart;
}

uint16_t C6502Introduction::GetLabelOffset() const
{
    return m_labOffset;
}

std::map<uint8_t, C6502Introduction> g_6502_Introduction_Set =
{
    //存取操作
    //====================================================================================================
    //LDA
    {(uint8_t)0xA1, C6502Introduction((uint8_t)0xA1, _T("LDA"), e6502AddressingMode::eModeIndexedIndirect)},

    {(uint8_t)0xA5, C6502Introduction((uint8_t)0xA5, _T("LDA"), e6502AddressingMode::eModeZeroPage)},
    {(uint8_t)0xA9, C6502Introduction((uint8_t)0xA9, _T("LDA"), e6502AddressingMode::eModeImmediate)},
    {(uint8_t)0xAD, C6502Introduction((uint8_t)0xAD, _T("LDA"), e6502AddressingMode::eModeAbsolute)},
    {(uint8_t)0xB1, C6502Introduction((uint8_t)0xB1, _T("LDA"), e6502AddressingMode::eModeIndirectIndexed)},
    {(uint8_t)0xB5, C6502Introduction((uint8_t)0xB5, _T("LDA"), e6502AddressingMode::eModeZeroPageX)},
    {(uint8_t)0xB9, C6502Introduction((uint8_t)0xB9, _T("LDA"), e6502AddressingMode::eModeAbsoluteY)},
    {(uint8_t)0xBD, C6502Introduction((uint8_t)0xBD, _T("LDA"), e6502AddressingMode::eModeAbsoluteX)},

    //LDX
    {(uint8_t)0xA2, C6502Introduction((uint8_t)0xA2, _T("LDX"), e6502AddressingMode::eModeImmediate)},
    {(uint8_t)0xA6, C6502Introduction((uint8_t)0xA6, _T("LDX"), e6502AddressingMode::eModeZeroPage)},
    {(uint8_t)0xAE, C6502Introduction((uint8_t)0xAE, _T("LDX"), e6502AddressingMode::eModeAbsolute)},
    {(uint8_t)0xB6, C6502Introduction((uint8_t)0xB6, _T("LDX"), e6502AddressingMode::eModeZeroPageY)},
    {(uint8_t)0xBE, C6502Introduction((uint8_t)0xBE, _T("LDX"), e6502AddressingMode::eModeAbsoluteY)},

    //LDY
    {(uint8_t)0xA0, C6502Introduction((uint8_t)0xA0, _T("LDY"), e6502AddressingMode::eModeImmediate)},
    {(uint8_t)0xA4, C6502Introduction((uint8_t)0xA4, _T("LDY"), e6502AddressingMode::eModeZeroPage)},
    {(uint8_t)0xAC, C6502Introduction((uint8_t)0xAC, _T("LDY"), e6502AddressingMode::eModeAbsolute)},
    {(uint8_t)0xB4, C6502Introduction((uint8_t)0xB4, _T("LDY"), e6502AddressingMode::eModeZeroPageX)},
    {(uint8_t)0xBC, C6502Introduction((uint8_t)0xBC, _T("LDY"), e6502AddressingMode::eModeAbsoluteX)},

    //STA
    {(uint8_t)0x81, C6502Introduction((uint8_t)0x81, _T("STA"), e6502AddressingMode::eModeIndexedIndirect)},
    {(uint8_t)0x85, C6502Introduction((uint8_t)0x85, _T("STA"), e6502AddressingMode::eModeZeroPage)},
    {(uint8_t)0x8D, C6502Introduction((uint8_t)0x8D, _T("STA"), e6502AddressingMode::eModeAbsolute)},
    {(uint8_t)0x91, C6502Introduction((uint8_t)0x91, _T("STA"), e6502AddressingMode::eModeIndirectIndexed)},
    {(uint8_t)0x95, C6502Introduction((uint8_t)0x95, _T("STA"), e6502AddressingMode::eModeZeroPageX)},
    {(uint8_t)0x9D, C6502Introduction((uint8_t)0x9D, _T("STA"), e6502AddressingMode::eModeAbsoluteX)},
    {(uint8_t)0x99, C6502Introduction((uint8_t)0x99, _T("STA"), e6502AddressingMode::eModeAbsoluteY)},

    //STX
    {(uint8_t)0x86, C6502Introduction((uint8_t)0x86, _T("STX"), e6502AddressingMode::eModeZeroPage)},
    {(uint8_t)0x96, C6502Introduction((uint8_t)0x96, _T("STX"), e6502AddressingMode::eModeZeroPageY)},
    {(uint8_t)0x8E, C6502Introduction((uint8_t)0x8E, _T("STX"), e6502AddressingMode::eModeAbsolute)},

    //STY
    {(uint8_t)0x84, C6502Introduction((uint8_t)0x84, _T("STY"), e6502AddressingMode::eModeZeroPage)},
    {(uint8_t)0x94, C6502Introduction((uint8_t)0x94, _T("STY"), e6502AddressingMode::eModeZeroPageX)},
    {(uint8_t)0x8C, C6502Introduction((uint8_t)0x8C, _T("STY"), e6502AddressingMode::eModeAbsolute)},

    //寄存器传送
    //====================================================================================================
    {(uint8_t)0xAA, C6502Introduction((uint8_t)0xAA, _T("TAX"), e6502AddressingMode::eModeImplicit)},
    {(uint8_t)0x8A, C6502Introduction((uint8_t)0x8A, _T("TXA"), e6502AddressingMode::eModeImplicit)},
    {(uint8_t)0xA8, C6502Introduction((uint8_t)0xA8, _T("TAY"), e6502AddressingMode::eModeImplicit)},
    {(uint8_t)0x98, C6502Introduction((uint8_t)0x98, _T("TYA"), e6502AddressingMode::eModeImplicit)},

    //栈操作
    //====================================================================================================
    {(uint8_t)0xBA, C6502Introduction((uint8_t)0xBA, _T("TSX"), e6502AddressingMode::eModeImplicit)},
    {(uint8_t)0x9A, C6502Introduction((uint8_t)0x9A, _T("TXS"), e6502AddressingMode::eModeImplicit)},
    {(uint8_t)0x48, C6502Introduction((uint8_t)0x48, _T("PHA"), e6502AddressingMode::eModeImplicit)},
    {(uint8_t)0x68, C6502Introduction((uint8_t)0x68, _T("PLA"), e6502AddressingMode::eModeImplicit)},
    {(uint8_t)0x08, C6502Introduction((uint8_t)0x08, _T("PHP"), e6502AddressingMode::eModeImplicit)},
    {(uint8_t)0x28, C6502Introduction((uint8_t)0x28, _T("PLP"), e6502AddressingMode::eModeImplicit)},

    //逻辑运算
    //====================================================================================================
    //AND
    {(uint8_t)0x21, C6502Introduction((uint8_t)0x21, _T("AND"), e6502AddressingMode::eModeIndexedIndirect)},
    {(uint8_t)0x25, C6502Introduction((uint8_t)0x25, _T("AND"), e6502AddressingMode::eModeZeroPage)},
    {(uint8_t)0x29, C6502Introduction((uint8_t)0x29, _T("AND"), e6502AddressingMode::eModeImmediate)},
    {(uint8_t)0x2D, C6502Introduction((uint8_t)0x2D, _T("AND"), e6502AddressingMode::eModeAbsolute)},
    {(uint8_t)0x31, C6502Introduction((uint8_t)0x31, _T("AND"), e6502AddressingMode::eModeIndirectIndexed)},
    {(uint8_t)0x35, C6502Introduction((uint8_t)0x35, _T("AND"), e6502AddressingMode::eModeZeroPageX)},
    {(uint8_t)0x39, C6502Introduction((uint8_t)0x39, _T("AND"), e6502AddressingMode::eModeAbsoluteY)},
    {(uint8_t)0x3D, C6502Introduction((uint8_t)0x3D, _T("AND"), e6502AddressingMode::eModeAbsoluteX)},

    //EOR
    {(uint8_t)0x41, C6502Introduction((uint8_t)0x41, _T("EOR"), e6502AddressingMode::eModeIndexedIndirect)},
    {(uint8_t)0x45, C6502Introduction((uint8_t)0x45, _T("EOR"), e6502AddressingMode::eModeZeroPage)},
    {(uint8_t)0x49, C6502Introduction((uint8_t)0x49, _T("EOR"), e6502AddressingMode::eModeImmediate)},
    {(uint8_t)0x4D, C6502Introduction((uint8_t)0x4D, _T("EOR"), e6502AddressingMode::eModeAbsolute)},
    {(uint8_t)0x51, C6502Introduction((uint8_t)0x51, _T("EOR"), e6502AddressingMode::eModeIndirectIndexed)},
    {(uint8_t)0x55, C6502Introduction((uint8_t)0x55, _T("EOR"), e6502AddressingMode::eModeZeroPageX)},
    {(uint8_t)0x59, C6502Introduction((uint8_t)0x59, _T("EOR"), e6502AddressingMode::eModeAbsoluteY)},
    {(uint8_t)0x5D, C6502Introduction((uint8_t)0x5D, _T("EOR"), e6502AddressingMode::eModeAbsoluteX)},

    //ORA
    {(uint8_t)0x01, C6502Introduction((uint8_t)0x01, _T("ORA"), e6502AddressingMode::eModeIndexedIndirect)},
    {(uint8_t)0x05, C6502Introduction((uint8_t)0x05, _T("ORA"), e6502AddressingMode::eModeZeroPage)},
    {(uint8_t)0x09, C6502Introduction((uint8_t)0x09, _T("ORA"), e6502AddressingMode::eModeImmediate)},
    {(uint8_t)0x0D, C6502Introduction((uint8_t)0x0D, _T("ORA"), e6502AddressingMode::eModeAbsolute)},
    {(uint8_t)0x11, C6502Introduction((uint8_t)0x11, _T("ORA"), e6502AddressingMode::eModeIndirectIndexed)},
    {(uint8_t)0x15, C6502Introduction((uint8_t)0x15, _T("ORA"), e6502AddressingMode::eModeZeroPageX)},
    {(uint8_t)0x19, C6502Introduction((uint8_t)0x19, _T("ORA"), e6502AddressingMode::eModeAbsoluteY)},
    {(uint8_t)0x1D, C6502Introduction((uint8_t)0x1D, _T("ORA"), e6502AddressingMode::eModeAbsoluteX)},

    //BIT
    {(uint8_t)0x24, C6502Introduction((uint8_t)0x24, _T("BIT"), e6502AddressingMode::eModeZeroPage)},
    {(uint8_t)0x2C, C6502Introduction((uint8_t)0x2C, _T("BIT"), e6502AddressingMode::eModeAbsolute)},

    //算术运算
    //====================================================================================================
    //ADC
    {(uint8_t)0x61, C6502Introduction((uint8_t)0x61, _T("ADC"), e6502AddressingMode::eModeIndexedIndirect) },
    {(uint8_t)0x65, C6502Introduction((uint8_t)0x65, _T("ADC"), e6502AddressingMode::eModeZeroPage) },
    {(uint8_t)0x69, C6502Introduction((uint8_t)0x69, _T("ADC"), e6502AddressingMode::eModeImmediate) },
    {(uint8_t)0x6D, C6502Introduction((uint8_t)0x6D, _T("ADC"), e6502AddressingMode::eModeAbsolute) },
    {(uint8_t)0x71, C6502Introduction((uint8_t)0x71, _T("ADC"), e6502AddressingMode::eModeIndirectIndexed) },
    {(uint8_t)0x75, C6502Introduction((uint8_t)0x75, _T("ADC"), e6502AddressingMode::eModeZeroPageX) },
    {(uint8_t)0x79, C6502Introduction((uint8_t)0x79, _T("ADC"), e6502AddressingMode::eModeAbsoluteY) },
    {(uint8_t)0x7D, C6502Introduction((uint8_t)0x7D, _T("ADC"), e6502AddressingMode::eModeAbsoluteX) },

    //SBC
    {(uint8_t)0xE1, C6502Introduction((uint8_t)0xE1, _T("SBC"), e6502AddressingMode::eModeIndexedIndirect) },
    {(uint8_t)0xE5, C6502Introduction((uint8_t)0xE5, _T("SBC"), e6502AddressingMode::eModeZeroPage) },
    {(uint8_t)0xE9, C6502Introduction((uint8_t)0xE9, _T("SBC"), e6502AddressingMode::eModeImmediate) },
    {(uint8_t)0xED, C6502Introduction((uint8_t)0xED, _T("SBC"), e6502AddressingMode::eModeAbsolute) },
    {(uint8_t)0xF1, C6502Introduction((uint8_t)0xF1, _T("SBC"), e6502AddressingMode::eModeIndirectIndexed) },
    {(uint8_t)0xF5, C6502Introduction((uint8_t)0xF5, _T("SBC"), e6502AddressingMode::eModeZeroPageX) },
    {(uint8_t)0xF9, C6502Introduction((uint8_t)0xF9, _T("SBC"), e6502AddressingMode::eModeAbsoluteY) },
    {(uint8_t)0xFD, C6502Introduction((uint8_t)0xFD, _T("SBC"), e6502AddressingMode::eModeAbsoluteX) },

    //CMP
    {(uint8_t)0xC1, C6502Introduction((uint8_t)0xC1, _T("CMP"), e6502AddressingMode::eModeIndexedIndirect) },
    {(uint8_t)0xC5, C6502Introduction((uint8_t)0xC5, _T("CMP"), e6502AddressingMode::eModeZeroPage) },
    {(uint8_t)0xC9, C6502Introduction((uint8_t)0xC9, _T("CMP"), e6502AddressingMode::eModeImmediate) },
    {(uint8_t)0xCD, C6502Introduction((uint8_t)0xCD, _T("CMP"), e6502AddressingMode::eModeAbsolute) },
    {(uint8_t)0xD1, C6502Introduction((uint8_t)0xD1, _T("CMP"), e6502AddressingMode::eModeIndirectIndexed) },
    {(uint8_t)0xD5, C6502Introduction((uint8_t)0xD5, _T("CMP"), e6502AddressingMode::eModeZeroPageX) },
    {(uint8_t)0xD9, C6502Introduction((uint8_t)0xD9, _T("CMP"), e6502AddressingMode::eModeAbsoluteY) },
    {(uint8_t)0xDD, C6502Introduction((uint8_t)0xDD, _T("CMP"), e6502AddressingMode::eModeAbsoluteX) },

    //CPX
    {(uint8_t)0xE0, C6502Introduction((uint8_t)0xE0, _T("CPX"), e6502AddressingMode::eModeImmediate) },
    {(uint8_t)0xE4, C6502Introduction((uint8_t)0xE4, _T("CPX"), e6502AddressingMode::eModeZeroPage) },
    {(uint8_t)0xEC, C6502Introduction((uint8_t)0xEC, _T("CPX"), e6502AddressingMode::eModeImmediate) },

    //CPY
    {(uint8_t)0xC0, C6502Introduction((uint8_t)0xC0, _T("CPY"), e6502AddressingMode::eModeImmediate) },
    {(uint8_t)0xC4, C6502Introduction((uint8_t)0xC4, _T("CPY"), e6502AddressingMode::eModeZeroPage) },
    {(uint8_t)0xCC, C6502Introduction((uint8_t)0xCC, _T("CPY"), e6502AddressingMode::eModeImmediate) },


    //自增/减操作
    //====================================================================================================
    //INC
    {(uint8_t)0xE6, C6502Introduction((uint8_t)0xC0, _T("INC"), e6502AddressingMode::eModeZeroPage) },
    {(uint8_t)0xEE, C6502Introduction((uint8_t)0xC0, _T("INC"), e6502AddressingMode::eModeAbsolute) },
    {(uint8_t)0xF6, C6502Introduction((uint8_t)0xC0, _T("INC"), e6502AddressingMode::eModeZeroPageX) },
    {(uint8_t)0xFE, C6502Introduction((uint8_t)0xC0, _T("INC"), e6502AddressingMode::eModeAbsoluteX) },

    //DEC
    {(uint8_t)0xC6, C6502Introduction((uint8_t)0xC6, _T("DEC"), e6502AddressingMode::eModeZeroPage) },
    {(uint8_t)0xCE, C6502Introduction((uint8_t)0xCE, _T("DEC"), e6502AddressingMode::eModeAbsolute) },
    {(uint8_t)0xD6, C6502Introduction((uint8_t)0xD6, _T("DEC"), e6502AddressingMode::eModeZeroPageX) },
    {(uint8_t)0xDE, C6502Introduction((uint8_t)0xDE, _T("DEC"), e6502AddressingMode::eModeAbsoluteX) },

    {(uint8_t)0xE8, C6502Introduction((uint8_t)0xE8, _T("INX"), e6502AddressingMode::eModeImplicit) },
    {(uint8_t)0xCA, C6502Introduction((uint8_t)0xCA, _T("DEX"), e6502AddressingMode::eModeImplicit) },
    {(uint8_t)0xC8, C6502Introduction((uint8_t)0xC8, _T("INY"), e6502AddressingMode::eModeImplicit) },
    {(uint8_t)0x88, C6502Introduction((uint8_t)0x88, _T("DEY"), e6502AddressingMode::eModeImplicit) },

    //移位操作
    //ASL
    {(uint8_t)0x06, C6502Introduction((uint8_t)0x06, _T("ASL"), e6502AddressingMode::eModeZeroPage) },
    {(uint8_t)0x0A, C6502Introduction((uint8_t)0x0A, _T("ASL"), e6502AddressingMode::eModeAccumulator) },
    {(uint8_t)0x0E, C6502Introduction((uint8_t)0x0E, _T("ASL"), e6502AddressingMode::eModeAbsolute) },
    {(uint8_t)0x16, C6502Introduction((uint8_t)0x16, _T("ASL"), e6502AddressingMode::eModeZeroPageX) },
    {(uint8_t)0x1E, C6502Introduction((uint8_t)0x1E, _T("ASL"), e6502AddressingMode::eModeAbsoluteX) },

    //LSR
    {(uint8_t)0x46, C6502Introduction((uint8_t)0x46, _T("LSR"), e6502AddressingMode::eModeZeroPage) },
    {(uint8_t)0x4A, C6502Introduction((uint8_t)0x4A, _T("LSR"), e6502AddressingMode::eModeAccumulator) },
    {(uint8_t)0x4E, C6502Introduction((uint8_t)0x4E, _T("LSR"), e6502AddressingMode::eModeAbsolute) },
    {(uint8_t)0x56, C6502Introduction((uint8_t)0x56, _T("LSR"), e6502AddressingMode::eModeZeroPageX) },
    {(uint8_t)0x5E, C6502Introduction((uint8_t)0x5E, _T("LSR"), e6502AddressingMode::eModeAbsoluteX) },

    //ROL
    {(uint8_t)0x26, C6502Introduction((uint8_t)0x26, _T("ROL"), e6502AddressingMode::eModeZeroPage) },
    {(uint8_t)0x2A, C6502Introduction((uint8_t)0x2A, _T("ROL"), e6502AddressingMode::eModeAccumulator) },
    {(uint8_t)0x2E, C6502Introduction((uint8_t)0x2E, _T("ROL"), e6502AddressingMode::eModeAbsolute) },
    {(uint8_t)0x36, C6502Introduction((uint8_t)0x36, _T("ROL"), e6502AddressingMode::eModeZeroPageX) },
    {(uint8_t)0x3E, C6502Introduction((uint8_t)0x3E, _T("ROL"), e6502AddressingMode::eModeAbsoluteX) },

    //ROR
    {(uint8_t)0x66, C6502Introduction((uint8_t)0x66, _T("ROR"), e6502AddressingMode::eModeZeroPage) },
    {(uint8_t)0x6A, C6502Introduction((uint8_t)0x6A, _T("ROR"), e6502AddressingMode::eModeAccumulator) },
    {(uint8_t)0x6E, C6502Introduction((uint8_t)0x6E, _T("ROR"), e6502AddressingMode::eModeAbsolute) },
    {(uint8_t)0x76, C6502Introduction((uint8_t)0x76, _T("ROR"), e6502AddressingMode::eModeZeroPageX) },
    {(uint8_t)0x7E, C6502Introduction((uint8_t)0x7E, _T("ROR"), e6502AddressingMode::eModeAbsoluteX) },

    //跳转与调用子程序
    {(uint8_t)0x4C, C6502Introduction((uint8_t)0x4C, _T("JMP"), e6502AddressingMode::eModeAbsolute) },
    {(uint8_t)0x6C, C6502Introduction((uint8_t)0x6C, _T("JMP"), e6502AddressingMode::eModeIndirect) },
    {(uint8_t)0x20, C6502Introduction((uint8_t)0x20, _T("JSR"), e6502AddressingMode::eModeAbsolute) },
    {(uint8_t)0x60, C6502Introduction((uint8_t)0x60, _T("RTS"), e6502AddressingMode::eModeImplicit) },

    //分支跳转
    {(uint8_t)0xF0, C6502Introduction((uint8_t)0xF0, _T("BEQ"), e6502AddressingMode::eModeRelative) },
    {(uint8_t)0xD0, C6502Introduction((uint8_t)0xD0, _T("BNE"), e6502AddressingMode::eModeRelative) },
    {(uint8_t)0xB0, C6502Introduction((uint8_t)0xB0, _T("BCS"), e6502AddressingMode::eModeRelative) },
    {(uint8_t)0x90, C6502Introduction((uint8_t)0x90, _T("BCC"), e6502AddressingMode::eModeRelative) },
    {(uint8_t)0x70, C6502Introduction((uint8_t)0x70, _T("BVS"), e6502AddressingMode::eModeRelative) },
    {(uint8_t)0x50, C6502Introduction((uint8_t)0x50, _T("BVC"), e6502AddressingMode::eModeRelative) },
    {(uint8_t)0x30, C6502Introduction((uint8_t)0x30, _T("BMI"), e6502AddressingMode::eModeRelative) },
    {(uint8_t)0x10, C6502Introduction((uint8_t)0x10, _T("BPL"), e6502AddressingMode::eModeRelative) },

    //状态标志操作
    {(uint8_t)0x18, C6502Introduction((uint8_t)0x18, _T("CLC"), e6502AddressingMode::eModeImplicit) },
    {(uint8_t)0x38, C6502Introduction((uint8_t)0x38, _T("SEC"), e6502AddressingMode::eModeImplicit) },
    {(uint8_t)0x58, C6502Introduction((uint8_t)0x58, _T("CLI"), e6502AddressingMode::eModeImplicit) },
    {(uint8_t)0x78, C6502Introduction((uint8_t)0x78, _T("SEI"), e6502AddressingMode::eModeImplicit) },
    {(uint8_t)0xD8, C6502Introduction((uint8_t)0xD8, _T("CLD"), e6502AddressingMode::eModeImplicit) },
    {(uint8_t)0xF8, C6502Introduction((uint8_t)0xF8, _T("SED"), e6502AddressingMode::eModeImplicit) },
    {(uint8_t)0xB8, C6502Introduction((uint8_t)0xB8, _T("CLV"), e6502AddressingMode::eModeImplicit) },
    
    //系统功能指令
    {(uint8_t)0x00, C6502Introduction((uint8_t)0x00, _T("BRK"), e6502AddressingMode::eModeImplicit) },
    {(uint8_t)0xEA, C6502Introduction((uint8_t)0xEA, _T("NOP"), e6502AddressingMode::eModeImplicit) },
    {(uint8_t)0x40, C6502Introduction((uint8_t)0x40, _T("RTI"), e6502AddressingMode::eModeImplicit) },
};