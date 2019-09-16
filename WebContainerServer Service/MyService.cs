using SimpleHttpServer;
using WebContainerServer;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Linq;
using System.ServiceProcess;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Timers;

namespace WebContainerServer_Service
{
    public partial class MyService : ServiceBase
    {
        HttpServer httpServer;
        Thread thread;
        EventLog eventLog = new EventLog("System");
        private int eventId = 1;

        public MyService()
        {
            InitializeComponent();
        }

        protected override void OnStart(string[] args)
        {
            httpServer = new HttpServer(3010, Routes.GET);

            thread = new Thread(new ThreadStart(httpServer.Listen));
            thread.Start();
            Thread.Sleep(1000);

            // Set up a timer that triggers every minute.
            //System.Timers.Timer timer = new System.Timers.Timer();
            //timer.Interval = 60000; // 60 seconds
            //timer.Elapsed += new ElapsedEventHandler(this.OnTimer);
            //timer.Start();
        }

        protected override void OnStop()
        {
            Environment.Exit(0);
        }

        public void OnTimer(object sender, ElapsedEventArgs args)
        {
            // TODO: Insert monitoring activities here.
            eventLog.WriteEntry("Monitoring the System(WebContainerServer Service)", EventLogEntryType.Information, eventId++);
        }

        internal void TestStartupAndStop(string[] args)
        {
            this.OnStart(args);
            Console.Write("Input Any Key : ");
            Console.ReadLine();
            this.OnStop();
        }
    }
}
