// EditStream.h
#pragma once
#include <streambuf>
#include <windows.h>
#include <string>
#include <mutex>
    class EditStreamBuf : public std::streambuf {
    public:
        EditStreamBuf(CEdit* pEdit) : m_pEdit(pEdit) {
            setp(buffer, buffer + sizeof(buffer) - 1); // 设置输出缓冲区
        }

    protected:
        static const int BUFFER_SIZE = 1024;
        char buffer[BUFFER_SIZE];
        CEdit* m_pEdit;

        virtual int overflow(int c) override {
            if (c != EOF) {
                *pptr() = c;
                pbump(1);
            }
            if (sync() == -1) return EOF;
            return c;
        }

        virtual int sync() override {
            if (pbase() == pptr()) return 0; // 缓冲区为空
            *pptr() = '\0'; // 添加字符串终止符
            CString str(pbase());

            if (m_pEdit) {
                int len = m_pEdit->GetWindowTextLength();
                m_pEdit->SetSel(len, len); // 移动光标到末尾
                m_pEdit->ReplaceSel(str);  // 追加文本
            }

            setp(buffer, buffer + BUFFER_SIZE - 1); // 重置缓冲区指针
            return 0;
        }
    };