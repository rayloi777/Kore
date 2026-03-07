let project = new Project('Triangle');

await project.addProject(findKore());

project.addFile('sources/**');
project.addKongDir('shaders');
project.setDebugDir('deployment');

project.addIncludeDir('../../build/Kong-osx-metal-triangle');
project.addFile('../../build/Kong-osx-metal-triangle/kong.m');

project.flatten();

resolve(project);
