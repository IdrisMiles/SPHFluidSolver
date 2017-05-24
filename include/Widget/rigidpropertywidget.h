#ifndef RIGIDPROPERTYWIDGET_H
#define RIGIDPROPERTYWIDGET_H

#include <QWidget>
#include "Widget/sphparticlepropertywidget.h"

#include "SPH/rigidproperty.h"

namespace Ui {
class RigidPropertyWidget;
}

class RigidPropertyWidget : public SphParticlePropertyWidget
{
    Q_OBJECT

public:
    explicit RigidPropertyWidget(QWidget *parent = 0, RigidProperty _property = RigidProperty(),
                                 float posX = 0.0f, float posY = 0.0f, float posZ = 0.0f, float rotX = 0.0f, float rotY = 0.0f, float rotZ = 0.0f);
    ~RigidPropertyWidget();

    virtual void SetProperty(RigidProperty _property);
    virtual RigidProperty GetProperty();

    void SetTransform(float posX, float posY, float posZ, float rotX, float rotY, float rotZ);

signals:
    /// @brief Qt Signal to communicate that the FluidProperty has changed to other classes
    void PropertyChanged(RigidProperty _property);
    void TransformChanged(float posX, float posY, float posZ, float rotX, float rotY, float rotZ, float scaleX, float scaleY, float scaleZ);

public slots:
    /// @brief Qt Slot to be connected to any changes on this widget, emits PropertyChanged(m_property)
    virtual void OnPropertyChanged();
    void OnTransformChanged();

private:
    Ui::RigidPropertyWidget *ui;

    RigidProperty m_property;
};

#endif // RIGIDPROPERTYWIDGET_H
