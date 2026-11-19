#pragma once
#include <stdint.h>
#include <string>
#include <map>
#include <vector>
#include <tchar.h>

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

//寻址模式
enum e6502AddressingMode
{
    eModeImplicit,          //隐含寻址
    eModeAccumulator,       //累加器寻址
    eModeImmediate,         //立即寻址
    eModeZeroPage,          //零页寻址
    eModeZeroPageX,         //零页寻址X变址
    eModeZeroPageY,         //零页寻址Y变址
    eModeRelative,          //相对寻址
    eModeAbsolute,          //绝对寻址
    eModeAbsoluteX,         //绝对寻址X变址
    eModeAbsoluteY,         //绝对寻址Y变址
    eModeIndirect,          //间接寻址
    eModeIndexedIndirect,   //先变址寻址后间接寻址
    eModeIndirectIndexed,   //先间接寻址后变址寻址
    eModeReserved,          //保留作为其他用途
};

typedef struct _DATA_INFO
{
    uint16_t m_addr = (uint16_t)0;
    uint16_t m_length = (uint16_t)0;
    uint8_t  m_elem = 1;

    _DATA_INFO(uint16_t addr, uint16_t length, uint8_t elem)
        :
        m_addr(addr),
        m_length(length),
        m_elem(elem)
    {

    }

}DATA_INFO;

//指令信息
class C6502Introduction
{
public:

    C6502Introduction(
        uint8_t op,
        const _tstring& strName,
        e6502AddressingMode type,
        uint16_t addr = (uint16_t)0,
        uint16_t data = (uint16_t)0,
        uint8_t elem = (uint16_t)1,
        bool bLabbeled = false,
        bool bReserved = false
    )
        : m_op(op), 
        m_name(strName), 
        m_type(type), 
        m_addr(addr), 
        m_data(data),
        m_elem(elem),
        m_bHasId(bLabbeled), 
        m_bReserved(bReserved),
        m_id((uint16_t)-1),
        m_refId((uint16_t)-1),
        m_labStart(0),
        m_labOffset(0)
    {
    }

    //获取指令名
    _tstring GetName() const;

    //获取寻址模式
    e6502AddressingMode GetAddressingMode() const;

    //获取指令格式
    _tstring GetFormat(const _tstring& strLabel = _T(""), bool b6502SimulatorSyntax = false, bool bShowAddr = false) const;

    //获取操作数长度
    uint8_t GetDataSize() const;

    //获取指令总长度
    uint8_t GetSize() const;

    //获取指令地址
    uint16_t GetAddr() const;

    //获取下一个指令地址
    uint16_t GetNextAddr() const;

    uint8_t GetOpCode() const { return m_op; };

    //获取指令数据
    uint16_t GetData() const;

    //获取元素大小
    uint16_t GetElemSize() const;

    //设置元素大小
    void SetElemSize(uint8_t elem);

    //获取调用地址
    uint16_t GetCallAddr() const;

    //是否存取绝对地址
    bool IsAbsoluteRead() const;

    //是否为JSR或JMP调用地址(绝对寻址, 不包含JMP间接寻址)
    bool IsJsrOrJmpAbsolute() const;

    //是否为JMP绝对寻址
    bool IsJmpAbsolute() const;

    //是否为Jsr绝对寻址
    bool IsJsrAbsolute() const;

    //是否调用相对寻址
    bool IsRelative() const;

    //是否标签化
    bool IsLabbeled() const;

    //是否保留
    bool IsReserved() const;

    //设置保留状态
    void SetReserved(bool bData);

    //设置标签化
    void SetLabbeled(bool bLabled);

    //获取标签ID
    uint16_t GetLabelID() const;

    //设置标签ID
    void SetLabelID(uint16_t id);

    //获取调用标签ID
    uint16_t GetRefID() const;

    //设置调用标签ID
    void SetRefID(uint16_t id);

    //设置指令地址
    void SetAddr(uint16_t addr);

    //设置操作数
    void SetData(uint16_t data);

    //设置调用标签偏移
    void SetLabelOffset(uint16_t data, uint8_t offset);

    uint16_t GetLabelAddr() const;

    //获取标签偏移
    uint16_t GetLabelOffset() const;

private:
    _tstring m_name;                //指令名
    uint16_t m_addr;                //指令地址
    uint8_t m_op;                   //操作码
    uint16_t m_data;                //操作数
    uint8_t m_elem;                 //元素大小
    uint16_t m_id;                  //标签ID
    uint16_t m_refId;               //引用的标签ID

    uint16_t m_labStart;            //标签起始地址
    uint16_t m_labOffset;           //标签偏移

    bool m_bHasId;                  //标签化
    bool m_bReserved;               //数据保留
    e6502AddressingMode m_type;     //寻址类型
};

//
// @brief: 获取反汇编指令列表
// @param: introductionList     存放指令列表
// @param: dataList             保留的数据列表
// @param: vAddr                编译地址列表
// @param: pBaseAddr            反汇编数据地址
// @param: size                 数据地址大小
// @ret: void
void DisassembleInstructions(
    std::map<uint16_t, C6502Introduction>& introductionList,
    std::map<uint16_t, DATA_INFO>& dataList,
    std::vector<uint16_t> vAddr,
    uint8_t* pBaseAddr,
    uint32_t size
);

//
// @brief: 获取反汇编指令列表
// @param: introductionList     存放指令列表
// @param: strLabel             标签名
// @ret: _tstring               字符串
_tstring GetDisassembleText(
    std::map<uint16_t, C6502Introduction>& introductionList,
    const _tstring& strLabel,
    uint8_t* pBaseAddr,
    uint32_t size,
    bool bShowAddr = true,
    bool b6502SimulatorSyntax = false
);


extern std::map<uint8_t, C6502Introduction> g_6502_Introduction_Set;