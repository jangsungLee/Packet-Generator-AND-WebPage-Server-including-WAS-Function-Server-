using SimpleHttpServer;
using WebContainerServer;
using System;
using System.Collections.Generic;
using System.Linq;
using System.ServiceProcess;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace WebContainerServer_Service
{
    static class Program
    {
        /// <summary>
        /// 해당 애플리케이션의 주 진입점입니다.
        /// </summary>
        static void Main()
        {
            ServiceBase[] ServicesToRun;
            ServicesToRun = new ServiceBase[]
            {
                new MyService()
            };
            ServiceBase.Run(ServicesToRun);
        }

        /// <summary>
        /// 해당 애플리케이션의 디버깅용 주 진입점입니다.
        /// </summary>
        //static void Main(string[] args)
        //{
        //    MyService service = new MyService();
        //    service.TestStartupAndStop(args);
        //    service.TestStartupAndStop(args);
        //    service.TestStartupAndStop(args);
        //}
    }
}
