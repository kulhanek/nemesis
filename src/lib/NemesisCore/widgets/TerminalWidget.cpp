// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2010 Petr Kulhanek, kulhanek@chemi.muni.cz
//
//     This program is free software; you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation; either version 2 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License along
//     with this program; if not, write to the Free Software Foundation, Inc.,
//     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// =============================================================================

#include <TerminalWidget.hpp>
#include <TerminalEdit.hpp>

#include <QVBoxLayout>
#include <QKeyEvent>


//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CTerminalWidget::CTerminalWidget(QWidget* p_parent,const QString& prompt)
    : QWidget(p_parent)
{
    PromptPos = 0;
    HistoryPos = 0;
    Prompt = prompt;

    BackgroundColor = QColor("black");
    ForegroundColor = QColor("white");

    EditArea = new CTerminalEdit(this);
    EditArea->setFrameStyle(QFrame::NoFrame);
    UpdateStyleSheet();

    QFont font;
    font.setFamily("Monospace");
    EditArea->setFont(font);

   // connect(EditArea,SIGNAL(cursorPositionChanged(void)),
   //        this,SLOT(CursorPositionChanged(void)));

    QVBoxLayout* p_layout = new QVBoxLayout(this);
    p_layout->addWidget(EditArea);
    p_layout->setContentsMargins(0,0,0,0);
    setLayout(p_layout);

    PrintPrompt();
}

// -----------------------------------------------------------------------------

CTerminalWidget::~CTerminalWidget(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CTerminalWidget::SetPrompt(const QString& text)
{
    Prompt = text;
}

//------------------------------------------------------------------------------

void CTerminalWidget::UpdateStyleSheet(void)
{
    QString sh;
    sh = "QTextEdit { background-color: " + BackgroundColor.name() + "; color: " + ForegroundColor.name() + "; }";
    EditArea->setStyleSheet(sh);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CTerminalWidget::PrintPrompt(void)
{
    QTextCursor cursor(EditArea->textCursor());
    cursor.clearSelection();
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(Prompt);

    PromptPos = cursor.position();
    EditArea->setTextCursor(cursor);
}

//------------------------------------------------------------------------------

// this part is rewritten version of pythonterminal.cpp from Avogadro

bool CTerminalWidget::KeyPressEvent(QKeyEvent* p_event)
{
    CheckCursorPosition();

    if( p_event->key() == Qt::Key_Up ) {
        if( History.size() ) {
            if( HistoryPos < 0 ) {
                p_event->accept();
                return(true);
            }
            HistoryPos--;

            if( HistoryPos < 0 ) {
                // we've reached the first command, display empty prompt
                QTextCursor cursor(EditArea->textCursor());
                cursor.movePosition(QTextCursor::End);
                cursor.setPosition(PromptPos,QTextCursor::KeepAnchor);
                cursor.removeSelectedText();
            } else {
                // display cached command
                QTextCursor cursor(EditArea->textCursor());
                cursor.movePosition(QTextCursor::End);
                cursor.setPosition(PromptPos,QTextCursor::KeepAnchor);
                cursor.removeSelectedText();
                cursor.insertText(History.at(HistoryPos));
                cursor.movePosition(QTextCursor::End);
                EditArea->setTextCursor(cursor);
            }

        }
        p_event->accept();
        return(true);
    }

    if( p_event->key() == Qt::Key_Down ) {
        if( History.size()) {
            if( HistoryPos >= History.size() ) {
                p_event->accept();
                return(true);
            }
            HistoryPos++;

            if( HistoryPos == History.size() ) {
                // we've reached the last command, display empty prompt
                QTextCursor cursor(EditArea->textCursor());
                cursor.movePosition(QTextCursor::End);
                cursor.setPosition(PromptPos,QTextCursor::KeepAnchor);
                cursor.removeSelectedText();
                EditArea->setTextCursor(cursor);
            } else {
                // display cached command
                QTextCursor cursor(EditArea->textCursor());
                cursor.movePosition(QTextCursor::End);
                cursor.setPosition(PromptPos,QTextCursor::KeepAnchor);
                cursor.removeSelectedText();
                cursor.insertText(History.at(HistoryPos));
                cursor.movePosition(QTextCursor::End);
                EditArea->setTextCursor(cursor);
            }

        }
        p_event->accept();
        return(true);
    }

    if( p_event->key() == Qt::Key_Backspace ) {
        QTextCursor cursor(EditArea->textCursor());
        if( cursor.position() <= PromptPos ) {
            // this has to be handled separatelly, because CursorPositionChanged
            // leads to SIGV
            p_event->accept();
            return(true);
        }
    }

    if( p_event->key() == Qt::Key_Return ) {
        QString line = EditArea->toPlainText();
        QString command = line.right(line.size() - PromptPos);

        if( ! command.isEmpty() ) {
            History.append(command);
            // this limits how many commands we save
            if(History.size() > 100) {
                History.removeFirst();
            }
        }
        HistoryPos = History.size();

        QTextCursor cursor(EditArea->textCursor());
        cursor.movePosition(QTextCursor::End);
        cursor.insertText("\n");
        cursor.movePosition(QTextCursor::End);
        EditArea->setTextCursor(cursor);

        emit OnNewLineEntered(command);

        PrintPrompt();
        p_event->accept();
        return(true);
    }

    return(false);
}

//------------------------------------------------------------------------------

bool CTerminalWidget::DragDropEvent(QDropEvent* p_event)
{

    QTextCursor cursor(EditArea->textCursor());
    cursor.setPosition(PromptPos);
    QRect start_cursor_rect = EditArea->cursorRect(cursor);
    if ( (p_event->type() == QMouseEvent::Drop) &&
        (p_event->pos().y() > start_cursor_rect.top()) &&
        (p_event->pos().x() > start_cursor_rect.left()) ) {
       return (false);
    }
    return (true);
}

//------------------------------------------------------------------------------

bool CTerminalWidget::MouseReleaseEvent(QMouseEvent* p_event)
{

    // read cursor position (position of cursor next to promt)
    QTextCursor cursor(EditArea->textCursor());
    cursor.setPosition(PromptPos);
    QRect start_cursor_rect = EditArea->cursorRect(cursor);

    if( (p_event->button() == Qt::MiddleButton) &&
        (p_event->y() > start_cursor_rect.top()) &&
        (p_event->x() > start_cursor_rect.left()) ) {
       return (false);
    }
    return (true);
}

//------------------------------------------------------------------------------

void CTerminalWidget::CheckCursorPosition(void)
{
    QTextCursor cursor(EditArea->textCursor());
    if( cursor.position() < PromptPos ) {
        cursor.setPosition(PromptPos);
        EditArea->setTextCursor(cursor);
    }
}

//------------------------------------------------------------------------------

void CTerminalWidget::CursorPositionChanged(void)
{
    QTextCursor cursor(EditArea->textCursor());
    if( cursor.position() < PromptPos ) {
        cursor.setPosition(PromptPos);
        EditArea->setTextCursor(cursor);
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
