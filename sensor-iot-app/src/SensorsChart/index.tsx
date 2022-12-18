/* eslint-disable react/no-array-index-key */
/* eslint-disable @typescript-eslint/ban-types */
import React, { useEffect } from 'react';
import Highcharts from 'highcharts';
import HighchartsExporting from 'highcharts/modules/exporting';
import PieChart from 'highcharts-react-official';
import { Box, Flex } from '@chakra-ui/react';

/* interface InvestmentChartProps {
  data: InvestmentChartData;
} */

const colorsChart = {
  Secondary: {
    '01': '#5cb879',
    '02': '#6fde92',
    '03': '#a3fbbf',
    '04': '#E3FFE9',
    '05': '#F1FCF3',
    500: '#5cb879'
  },
  Green: {
    '01': '#1F963A',
    '02': '#2FC350',
    '03': '#7EDF94',
    '04': '#D2F6DA'
  },
}
export default function SensorsChart( historicData : any ) {    
  const chartData = historicData.chartData;
  if (typeof Highcharts === 'object') {
    HighchartsExporting(Highcharts);
  }

  const colors = [
    colorsChart.Secondary['01'],
    colorsChart.Green['04'],
    colorsChart.Green['01'],
    colorsChart.Green['02'],
    colorsChart.Secondary['01'],
    colorsChart.Secondary['02']
  ];

  const options: Highcharts.Options = {    
    chart: {
      type: 'area-spline',
      backgroundColor: 'transparent'
    },
    title: {
      text: ''
    },
    colors,

    credits: {
      enabled: false
    },

    navigation: {
      buttonOptions: {
        enabled: false
      }
    },    
    plotOptions: {        
      areaspline: {
        allowPointSelect: false,
        cursor: 'pointer',
        dataLabels: {
          enabled: true,
          format: '{point.percentage:.1f} %',          
          style: {
            color: 'white',
            fontSize: '14.2.px',
            fontFamily: "'Urbane', 'public sans'",
            fontWeight: '500',
            lineHeight: '16.98px',
            textOutline: '0px'
          }
        },
        showInLegend: false
      }
    },    
    yAxis:{},
    series: [
      {
        type: 'areaspline',
        name: 'Quantidade',
        fillColor: {
          linearGradient: {
            x1: 0,
            x2: 0,
            y1: 0,
            y2: 1
          },
          stops: [
            [0, 'rgba(163, 251, 191, 0.8)'],
            [1, 'rgba(255, 255, 255, 0.15)']
          ]
        },        
        pointStart: 0,
        data: [
            [1, chartData[0]], 
            [2, chartData[1]],
            [3, chartData[2]], 
            [4, chartData[3]], 
            [5, chartData[4]], 
            [6, chartData[5]], 
            [7, chartData[6]], 
            [8, chartData[7]],
            [9, chartData[8]],
            [10, chartData[9]]
        ]
      }
    ]
  };
  return (
    <Flex flexDirection="row">
      <Box width="100%" height="100%">
        <PieChart highcharts={Highcharts} options={options} />
      </Box>
    </Flex>
  );
}
