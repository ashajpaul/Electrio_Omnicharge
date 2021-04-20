package com.example.myfirstapp;

import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import com.jjoe64.graphview.GraphView;
import com.jjoe64.graphview.series.DataPoint;
import com.jjoe64.graphview.series.LineGraphSeries;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        /*
        //Getting graph from layout
        GraphView graph = (GraphView) findViewById(R.id.graph);

        //Making series
        LineGraphSeries<DataPoint> series = new LineGraphSeries<>();
        double y;
        for(int x = 0; x < 90; x++)
        {
            y = Math.sin(2*x*0.2) - 2*Math.sin(x*0.2);
            series.appendData(new DataPoint(x,y), true, 90);
        }
        graph.addSeries(series);
         */

        GraphView graph = (GraphView) findViewById(R.id.graph);
        LineGraphSeries<DataPoint> series = new LineGraphSeries<>(new DataPoint[] {
                new DataPoint(0, 1),
                new DataPoint(1, 5),
                new DataPoint(2, 3),
                new DataPoint(3, 2),
                new DataPoint(4, 6)
        });
        graph.addSeries(series);
    }
}