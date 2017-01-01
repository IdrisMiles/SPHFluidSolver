#ifndef FLUIDPROPERTYWIDGET_H
#define FLUIDPROPERTYWIDGET_H

#include <QGroupBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QGridLayout>

#include <memory>
#include "fluidproperty.h"

class FluidPropertyWidget : public QWidget
{
public:
    FluidPropertyWidget(QWidget *parent = nullptr);
    ~FluidPropertyWidget();

    void SetFluidProperty(std::shared_ptr<FluidProperty> _fluidProperty);
    std::shared_ptr<FluidProperty> GetFluidProperty();


private:
    std::shared_ptr<FluidProperty> m_fluidProperty;

    std::shared_ptr<QGridLayout> m_gridLayout;

    std::shared_ptr<QDoubleSpinBox> numParticles;
    std::shared_ptr<QDoubleSpinBox> smoothingLength;
    std::shared_ptr<QDoubleSpinBox> particleRadius;
    std::shared_ptr<QDoubleSpinBox> particleMass;
    std::shared_ptr<QDoubleSpinBox> restDensity;

    std::shared_ptr<QLabel> surfaceTensionLabel;
    std::shared_ptr<QDoubleSpinBox> surfaceTension;
    std::shared_ptr<QLabel> surfaceThresholdLabel;
    std::shared_ptr<QDoubleSpinBox> surfaceThreshold;
    std::shared_ptr<QLabel> gasStiffnessLabel;
    std::shared_ptr<QDoubleSpinBox> gasStiffness;
    std::shared_ptr<QLabel> viscosityLabel;
    std::shared_ptr<QDoubleSpinBox> viscosity;

    std::shared_ptr<QDoubleSpinBox> deltaTime;
    std::shared_ptr<QDoubleSpinBox> solveIterations;
    std::shared_ptr<QDoubleSpinBox> gridResolution;
    std::shared_ptr<QDoubleSpinBox> gridCellWidth;
};

#endif // FLUIDPROPERTYWIDGET_H
