#ifndef SOLVERPROPERTYWIDGET_H
#define SOLVERPROPERTYWIDGET_H

//--------------------------------------------------------------------------------------------------------------

#include <QWidget>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QPushButton>

#include <memory>

#include "FluidSystem/fluidsystem.h"
#include "FluidSystem/fluidsolverproperty.h"

//--------------------------------------------------------------------------------------------------------------
/// @author Idris Miles
/// @version 1.0
/// @date 01/06/2017
//--------------------------------------------------------------------------------------------------------------


namespace Ui {
class SolverPropertyWidget;
}

/// @class SolverProperyWidget
/// @brief Inherits from QWidget. Widget for manipulating FluidSolverProperty class.
class SolverPropertyWidget : public QWidget
{
    Q_OBJECT

public:
    /// @brief comnstructor
    explicit SolverPropertyWidget(QWidget *parent = 0, FluidSolverProperty _property = FluidSolverProperty());

    /// @brief destructor
    virtual ~SolverPropertyWidget();



    /// @brief Setter for the m_property attribute
    virtual void SetProperty(const FluidSolverProperty &_property);

    /// @brief Geter for the m_property attribute
    virtual FluidSolverProperty GetProperty() const;


signals:
    /// @brief Qt Signal to communicate that the FluidProperty has changed to other classes
    void PropertyChanged(const FluidSolverProperty &_property);

public slots:
    /// @brief Qt Slot to be connected to any changes on this widget, emits PropertyChanged(m_property)
    virtual void OnPropertyChanged();

private:
    Ui::SolverPropertyWidget *ui;

    FluidSolverProperty m_property;


};

//--------------------------------------------------------------------------------------------------------------

#endif // SOLVERPROPERTYWIDGET_H
